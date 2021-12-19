
; Note: signal_calller assumes that signal_handler function pointer is in EAX register
; and a signo is on top of the stack.
global signal_caller
global signal_caller_end
signal_caller:
    call eax
    mov eax, 119 ; sigreturn
    int 0x80
signal_caller_end:
