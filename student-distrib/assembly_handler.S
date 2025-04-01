#define asm

# TranslateAssembly(name,handler_function)
# This function will do assembly linkage on c functions
# Inputs   : name: name to call the new linked function, handler_function : old c function to link
# Outputs  : None, just links functions  
#define TranslateAssembly(name,handler_function)\
.globl	name                                           ;\
name:                                                  ;\
    PUSHAL                                             ;\
    PUSHFL                                             ;\
    call handler_function                              ;\
    POPFL                                              ;\
    POPAL                                              ;\
    iret                                               

# call the above linking code on our c keyboard_handler function and c rtc_handler function
TranslateAssembly(keyboard_translated,keyboard_handler);
TranslateAssembly(rtc_translated,rtc_handler);
TranslateAssembly(pit_translated,pit_handler);

