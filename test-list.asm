[section ram]
JMP main
CALL main
HLT
const1:dw 1
const2:dw 2
main:
PUSH_FROM const1
PUSH_FROM const2
ADD
PUSH 3
MUL
POP
RET
