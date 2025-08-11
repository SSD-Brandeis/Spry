import json

# Python dictionary containing data to be saved as JSON
data_to_save = {
    "name": "Alice",
    "age": 25,
    "email": "alice@example.com"
}

path = '/mnt/d/LocalLaptop/BU/courses/2023-Spring/CS_561/Project/LSMRangeDeletes/development/rocksdb/self_RD/workload/'

# # Open the JSON file for writing
# with open(path + 'output.json', 'w') as file:
#     json.dump(data_to_save, file)


# Open the JSON file for reading
# with open(path + 'workload1.txt.testing_log2', 'r') as file:
with open(path + 'output.json', 'r') as file:
    data = json.load(file)

# Now 'data' contains the JSON data loaded as a Python dictionary
print(data["A"])


json_string = '[{"name": "Bob", "age": 35, "email": "bob@example.com"}]'

# Convert the JSON string to a Python dictionary
python_data = json.loads(json_string)

print(python_data)
