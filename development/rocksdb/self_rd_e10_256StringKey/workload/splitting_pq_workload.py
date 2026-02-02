import os

# Split large workload file into rounds with non-overlapping chunks
# Each PQ level has 3 rounds, each round gets pq new lines.

pq_groups = [5000]
rounds_per_group = 3
# Adjusted to be relative to the root where the caller Script is running
input_file = os.path.join("workload", "pq_workload_on_currently_non_inserted_keys.txt")

if not os.path.exists(input_file):
    print(f"Error: {input_file} not found.")
    exit(1)

with open(input_file, "r", encoding="utf-8") as infile:
    line_iter = iter(infile)
    round_index = 0

    for pq in pq_groups:
        for r in range(rounds_per_group):
            output_filename = os.path.join(
                "workload",
                f"workload6111.txt_currently_non_inserted_keys_round_{r}_number_of_pq_{pq}"
            )

            with open(output_filename, "w", encoding="utf-8") as outfile:
                for _ in range(pq):
                    line = next(line_iter, None)
                    if line is None:
                        break
                    outfile.write(line)

            print(f"✅ Created {output_filename} with ~{pq} lines.")
            round_index += 1
