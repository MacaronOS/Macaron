import glob
import os

include = '''
#include <wisterialib/Serialization.hpp>
#include <wisterialib/String.hpp>
#include <wisterialib/common.hpp>
#include <wisterialib/extras.hpp>
'''

get_type = '''
MessageType GetType(const Vector<char>& buffer)
{
    return static_cast<MessageType>(Decoder(buffer).get_int());
}
'''


def write_message_data(message_data):
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

        out.write(f'\n\t{message_name}(const Vector<char>& buffer)\n')
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

        out.write('\n\tVector<char> serialize()\n')
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


files = glob.glob("**/*.wrp", recursive=True)

for filepath in files:
    outfilepath = os.path.splitext(filepath)[0] + '.hpp'

    with open(filepath) as file:

        with open(outfilepath, 'w') as out:

            message_data = []

            content = file.read()
            messages = content.split('\n\n')

            out.write('#pragma once\n')
            out.write(include)
            out.write('\n')

            for message_index, message in enumerate(messages):
                # the first line is namespace
                if message_index == 0:
                    out.write(f'{message} {{\n\n')
                    continue

                types = []
                names = []
                message_name = ''

                lines = message.split('\n')
                for index, line in enumerate(lines):
                    if index == 0:
                        message_name = line
                    else:
                        line_stripped = line.strip()
                        line_stripped = line_stripped.lstrip()
                        typ, name = line_stripped.split()
                        types.append(typ)
                        names.append(name)

                message_data.append((message_name, names, types))

            write_message_data(message_data)
