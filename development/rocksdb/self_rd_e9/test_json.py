import json
with open('./output_statistics/workload/workload1011.txt_during_insertion.json') as f:
    data = json.load(f)
print(data)