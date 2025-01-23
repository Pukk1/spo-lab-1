[section ram]
PUSH 1
PUSH 2
PUSH 3
CALL main
POP
HLT
const1:dw 0,0,0,1
const2:dw 0,0,0,2
main:
LOAD_CONST const1
LOAD_CONST const2
ADD
PUSH 3
MUL
RET 3,1
