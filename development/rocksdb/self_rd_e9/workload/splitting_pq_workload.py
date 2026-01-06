# Split large workload file into rounds with non-overlapping chunks
# Each PQ level has 3 rounds, each round gets pq new lines.

pq_groups = [100, 200, 300, 400, 500]
rounds_per_group = 3
input_file = "pq_workload_on_currently_non_inserted_keys.txt"

with open(input_file, "r", encoding="utf-8") as infile:
    line_iter = iter(infile)
    round_index = 0

    for pq in pq_groups:
        for r in range(rounds_per_group):
            output_filename = (
                f"workload6111.txt_currently_non_inserted_keys_round_{r}"
                f"_number_of_pq_{pq}"
            )

            with open(output_filename, "w", encoding="utf-8") as outfile:
                for _ in range(pq):
                    line = next(line_iter, None)
                    if line is None:
                        break
                    #outfile.write(line)
                    outfile.write(line)

            print(f"✅ Created {output_filename} with ~{pq} lines.")
            round_index += 1

