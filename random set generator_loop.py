import random
from collections import defaultdict
import matplotlib.pyplot as plt
import sys
import datetime

# Cross-platform single-keypress function
def get_keypress():
    try:
        # Windows
        import msvcrt
        return msvcrt.getch().decode('utf-8').lower()
    except ImportError:
        # Unix/Mac
        import tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch.lower()

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

        # For the first element, exclude 0
        if len(sequence) == 0:
            candidates = [v for v in values if v != 0]
        else:
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

    # Add a zero at the start
    sequence = [0] + sequence

    return sequence

# Parameters
values = [0, 1, 2, 3, 4]
iterations = 20
num_sequences = 30
output_file = "sequences.txt"

for i in range(num_sequences):
    #random.seed()  # Different seed for each sequence for variety
    sequence = generate_balanced_sequence(values, iterations)
    formatted = ", ".join(str(x) for x in sequence)
    print(f"Sequence {i+1}: [{formatted}]")
    plt.figure(figsize=(10, 3))
    plt.plot(sequence, marker='o', linestyle='-', color='C0')
    plt.title(f"Sequence {i+1}")
    plt.xlabel("Step")
    plt.xticks(range(iterations + 1))
    plt.ylabel("Value")
    plt.yticks(values)
    plt.grid(True)
    plt.tight_layout()
    plt.show()
    print("Store this sequence? (y/n): ", end='', flush=True)
    answer = get_keypress()
    print(answer)
    if answer == 'y':
        with open(output_file, "a") as f:
            f.write(f"int set[] = {{{formatted}}};\n")
        print("Sequence stored.\n")
    else:
        print("Sequence discarded.\n")