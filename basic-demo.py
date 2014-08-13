import grakopp.buffer
import basic

b = grakopp.buffer.PyBuffer()
b.from_string("e1e2")

p = basic.basicPyParser()
p.set_nameguard(False)
p.set_whitespace("")
p.set_buffer(b)

s = basic.basicPySemantics()
p.set_semantics(s)

a = p._sequence_()

print a.to_python()

