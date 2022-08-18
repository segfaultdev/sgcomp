# sgcomp

TODO: make beautiful readme :D

i - input: is address between 0xF000 and 0xF7FF?
o1 - output 1: is address between 0xF000 and 0xF3FF?
o2 - output 2: is address between 0xF400 and 0xF7FF?

o1 = i & ~a10
o2 = i & a10
