import random
from collections import defaultdict
import matplotlib.pyplot as plt

def generate_balanced_sequence(values, iterations):
    num_values = len(values)
    base_count = iterations // num_values
    extra = iterations % num_values

    target_counts = {v: base_count for v in values}
    for v in random.sample(values, extra):
        target_counts[v] += 1

    def backtrack(sequence, counts, last_value):
        if len(sequence) == iterations:
            return sequence

        candidates = [v for v in values if counts[v] < target_counts[v] and v != last_value]
        random.shuffle(candidates)

        for v in candidates:
            sequence.append(v)
            counts[v] += 1
            result = backtrack(sequence, counts, v)
            if result:
                return result
            sequence.pop()
            counts[v] -= 1

        return None

    counts = defaultdict(int)
    sequence = backtrack([], counts, None)

    if sequence is None:
        raise RuntimeError("Failed to generate a valid sequence with the given constraints.")

    return sequence

# Parameters
values = [1, 2, 3, 4, 5]
iterations = 20

# Generate sequence
sequence = generate_balanced_sequence(values, iterations)

# Print C-style array
formatted = ", ".join(str(x) for x in sequence)
print(f"\nint sequence[{iterations}] = {{{formatted}}};\n")

# Plot
plt.figure(figsize=(14, 4))
plt.plot(sequence, marker='o', linestyle='-', color='blue')
plt.title(f"Balanced Random Sequence of {iterations} Steps (No Repeats)")
plt.xlabel("Step")
plt.ylabel("Value")
plt.yticks(values)
plt.grid(True)
plt.tight_layout()
plt.show()
