import subprocess
import time
import re
from pathlib import Path

working_dir = Path("/tmp/cs561_project1")

#use subprocess to run string to each file before retreiving 000* number from the files
def get_file_number():
    files = []
    for file in working_dir.glob('**/*.sst'):
        files.append(file)
    # for file in working_dir.glob('**/*.log'):
    #     files.append(file)

    list_keys = []
    for file in files:
        print(file)
        # print(subprocess.run(["strings", file], stdout=subprocess.PIPE).stdout.decode('utf-8'))
        # print(subprocess.run(["strings", file], stdout=subprocess.PIPE).stdout.decode('utf-8').split("000")[1].split(" ")[0])
        
        #using regex to get the number, numbers are of 12-digit
        keys = re.findall(r'\d{12}', subprocess.run(["strings", file], stdout=subprocess.PIPE).stdout.decode('utf-8'))

        list_keys.extend(keys)
        print(len(keys))
        print(keys)

        list_keys.sort()
    return list_keys


list_keys = get_file_number()
list_keys = [int(key) for key in list_keys]

# for key in list_keys:
#     if(key < 8000):
#         print("key < 5000", key)
print("Checking done.")
# print(list_keys)
