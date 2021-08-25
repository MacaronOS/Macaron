import glob
import os

include = '''
#include <Macaronlib/Common.hpp>
#include <Macaronlib/Runtime.hpp>
#include <Macaronlib/Serialization.hpp>
#include <Macaronlib/String.hpp>
'''

get_type = '''
inline MessageType GetType(const Vector<unsigned char>& buffer)
{
    return static_cast<MessageType>(Decoder(buffer).get_int());
}
'''

client_to_server = 0
server_to_client = 1


def write_message_data(message_data, outfilepath, namespace):
    os.makedirs(os.path.dirname(outfilepath), exist_ok=True)
    with open(outfilepath, 'w+') as out:
        out.write('#pragma once\n')
        out.write(include)
        out.write('\n')
        out.write(f'{namespace} {{\n\n')

        out.write('enum class MessageType : int {\n');
        for data in message_data:
            out.write(f'\t{data[0]},\n')
        out.write('};\n')

        out.write(get_type)
        out.write('\n')

        for message_name, names, types in message_data:
            out.write(f'class {message_name} {{\n')
            out.write(f'\tstatic constexpr MessageType m_type = MessageType::{message_name};\n\n')
            out.write('public:\n')

            params = ''
            for i in range(len(types)):
                params += f'{types[i]} {names[i]}'
                if i != len(types) - 1:
                    params += ', '

            out.write(f'\t{message_name}({params})\n')
            for i in range(len(names)):
                typ, name = types[i], names[i]

                name_right = name if typ != 'String' else f'move({name})'

                if i == 0:
                    out.write(f'\t\t: m_{name}({name_right})\n')
                else:
                    out.write(f'\t\t, m_{name}({name_right})\n')
            out.write('\t{\n\t}\n')

            out.write(f'\n\t{message_name}(const Vector<unsigned char>& buffer)\n')
            out.write('\t{\n')
            out.write('\t\tDecoder decoder(buffer);\n')
            out.write('\t\tdecoder.skip(sizeof(MessageType));\n')
            for i in range(len(types)):
                typ, name = types[i], names[i]
                out.write(f'\t\tm_{name} = decoder.get_{typ}();\n')

            out.write('\t}\n\n')

            for i in range(len(types)):
                typ, name = types[i], names[i]

                if typ == 'String':
                    out.write(f'\tconst {typ}& {name}() const {{ return m_{name}; }}\n')
                    out.write(f'\t{typ} take_{name}() {{ return move(m_{name}); }}\n')
                else:
                    out.write(f'\t{typ} {name}() const {{ return m_{name}; }}\n')

            out.write('\n\tVector<unsigned char> serialize() const\n')
            out.write('\t{\n')
            out.write('\t\tEncoder encoder {};\n')
            out.write('\t\tencoder.push((int)m_type);\n')

            for i in range(len(types)):
                typ, name = types[i], names[i]
                out.write(f'\t\tencoder.push(m_{name});\n')

            out.write(f'\t\treturn encoder.done();\n')

            out.write('\t}\n')

            out.write('\nprivate:\n')
            for i in range(len(types)):
                typ, name = types[i], names[i]
                out.write(f'\t{typ} m_{name};\n')

            out.write('};\n\n')

        out.write('}\n')


def write_connection(
        to_data, from_data, outfilepath,
        namespace, server_or_client, messages_relative_path
):
    os.makedirs(os.path.dirname(outfilepath), exist_ok=True)
    with open(outfilepath, 'w') as out:
        out.write('#pragma once\n\n')
        out.write(f'#include "{messages_relative_path}"\n\n')
        out.write(f'#include <Libipc/{server_or_client}Connection.hpp>\n\n')

        out.write(f'{namespace} {{\n\n')
        out.write(f'class {server_or_client}MessageReciever {{\n')
        out.write('public:\n')
        for request, response in to_data:
            if server_or_client == 'Server':
                out.write(f'\tvirtual {response} on_{request}({request}& request, int pid_from);\n')
            else:
                out.write(f'\tvirtual {response} on_{request}({request}& request);\n')
        for request, response in from_data:
            if response != 'void':
                if server_or_client == 'Server':
                    out.write(f'\tvirtual void on_{response}({response}& response, int pid_from);\n')
                else:
                    out.write(f'\tvirtual void on_{response}({response}& response);\n')
        out.write('};\n\n')

        out.write(f'class {server_or_client}Connection : public IPC::{server_or_client}Connection {{\n')
        out.write('public:\n')
        out.write(f'\t{server_or_client}Connection(const String& endpoint, {server_or_client}MessageReciever& reciever)\n')
        out.write(f'\t\t: IPC::{server_or_client}Connection(endpoint)\n')
        out.write('\t\t, m_reciever(reciever)\n')
        out.write('\t{\n')
        out.write('\t}\n')

        out.write('\n\tvoid process_messages()\n')
        out.write('\t{\n')
        out.write('\t\tpump();\n')
        out.write('\t\tauto recieved_messages_bytes = take_over_messages();\n')
        if server_or_client == 'Server':
            out.write('\t\tfor (auto& server_message : recieved_messages_bytes) {\n')
            out.write('\t\t\tauto message_bytes = server_message.message;\n')
            out.write('\t\t\tauto message_pid = server_message.pid_from;\n')
        else:
            out.write('\t\tfor (auto& message_bytes : recieved_messages_bytes) {\n')

        out.write('\t\t\tauto type = GetType(message_bytes);\n')

        for request, response in to_data:
            out.write(f'\n\t\t\tif (type == MessageType::{request}) {{\n')
            out.write(f'\t\t\t\tauto message = {request}(message_bytes);\n')
            if response != 'void':
                if server_or_client == 'Server':
                    out.write(f'\t\t\t\tauto response = m_reciever.on_{request}(message, message_pid).serialize();\n')
                    out.write('\t\t\t\tsend_data(response.data(), response.size(), message_pid);\n')
                else:
                    out.write(f'\t\t\t\tauto response = m_reciever.on_{request}(message).serialize();\n')
                    out.write('\t\t\t\tsend_data(response.data(), response.size());\n')
            else:
                if server_or_client == 'Server':
                    out.write(f'\t\t\t\tm_reciever.on_{request}(message, message_pid);\n')
                else:
                    out.write(f'\t\t\t\tm_reciever.on_{request}(message);\n')
            out.write('\t\t\t}\n')

        for request, response in from_data:
            if response != 'void':
                out.write(f'\n\t\t\tif (type == MessageType::{response}) {{\n')
                out.write(f'\t\t\t\tauto message = {response}(message_bytes);\n')
                if server_or_client == 'Server':
                    out.write(f'\t\t\t\tm_reciever.on_{response}(message, message_pid);\n')
                else:
                    out.write(f'\t\t\t\tm_reciever.on_{response}(message);\n')
                out.write('\t\t\t}\n')

        out.write('\t\t}\n')
        out.write('\t}\n')

        for request, _ in from_data:
            if server_or_client == 'Server':
                out.write(f'\n\tvoid send_{request}(const {request}& request, int pid_to)\n')
            else:
                out.write(f'\n\tvoid send_{request}(const {request}& request)\n')
            out.write('\t{\n')
            out.write('\t\tauto serialized = request.serialize();\n')
            if server_or_client == 'Server':
                out.write('\t\tsend_data(serialized.data(), serialized.size(), pid_to);\n')
            else:
                out.write('\t\tsend_data(serialized.data(), serialized.size());\n')
            out.write('\t}\n')

        out.write('\nprivate:\n')
        out.write(f'\t{server_or_client}MessageReciever& m_reciever;\n')
        out.write('};\n\n')

        out.write('}')


def parse_args(line):
    args = line.strip('(').strip(')').split(',')
    args_names = []
    args_types = []
    for arg in args:
        if arg != '':
            type, name = arg.strip().lstrip().split()
            args_types.append(type)
            args_names.append(name)

    return args_names, args_types


if __name__ == '__main__':
    files = glob.glob("**/*.protocols", recursive=True)

    for filepath in files:
        messages_path = os.path.splitext(filepath)[0] + 'Protocols/Messages.hpp'
        server_connection_path = os.path.splitext(filepath)[0] + 'Protocols/ServerConnection.hpp'
        client_connection_path = os.path.splitext(filepath)[0] + 'Protocols/ClientConnection.hpp'
        messages_relative_path = os.path.basename(messages_path)

        with open(filepath) as file:
            content = file.read()
            connections = content.split('\n\n')

            namespace = ''
            message_data = []
            receivers_info = [[], []]

            for connection_index, connection in enumerate(connections):
                # the first line is namespace
                if connection_index == 0:
                    namespace = connection + '::Protocols'
                    continue

                connection_direction = 0

                for protocol_index, protocol in enumerate(connection.split('\n')):
                    if protocol_index == 0:
                        left, right = protocol.split('->')
                        left = left.strip()
                        right = right.lstrip().strip(':')
                        if left == 'Client' and right == 'Server':
                            connection_direction = client_to_server
                        else:
                            connection_direction = server_to_client
                    else:
                        request, response = protocol.split('->')
                        request = request.strip().lstrip()
                        response = response.strip().lstrip()

                        request_name, request_args = request.split('(')
                        request_args = parse_args(request_args)
                        message_data.append((request_name + 'Request', request_args[0], request_args[1]))

                        if response != 'void':
                            response_args = parse_args(response)
                            message_data.append((request_name + 'Response', response_args[0], response_args[1]))
                            receivers_info[connection_direction].append(
                                (request_name + 'Request', request_name + 'Response'))
                        else:
                            receivers_info[connection_direction].append((request_name + 'Request', 'void'))

            write_message_data(message_data, messages_path, namespace)
            write_connection(
                receivers_info[client_to_server], receivers_info[server_to_client],
                server_connection_path, namespace, 'Server', messages_relative_path
            )
            write_connection(
                receivers_info[server_to_client], receivers_info[client_to_server],
                client_connection_path, namespace, 'Client', messages_relative_path
            )
