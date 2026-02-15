import os

path = r'd:\LocalLaptop\BU\courses\2023-Spring\CS_561\Project\LSMRangeDeletes\development\rocksdb\include\rocksdb\sys_rdfilter.h'
with open(path, 'rb') as f:
    content = f.read()

# Replace \r\n with \n, then replace remaining \r with \n
content = content.replace(b'\r\n', b'\n')
content = content.replace(b'\r', b'\n')

with open(path, 'wb') as f:
    f.write(content)

print("Line endings normalized to LF")
