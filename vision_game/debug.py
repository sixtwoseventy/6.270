import os
try:
    os.unlink("tx") 
except Exception as e:
    pass

os.mkfifo("tx")

f  = open("tx", "r")

while 1:
    c = f.read(1)
    print "%x" % ord(c)


