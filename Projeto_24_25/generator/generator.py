import random
import os

# Constants
PLID = "123456"  # Example player ID for testing
MAX_PLAYTIME = 300  # Max playtime for start and debug commands (between 0 and 600)
COLORS = ['R', 'G', 'B', 'Y', 'O', 'P']
OUTPUT_FILE = "test_commands.txt"  # Single output file

# Function to generate a random guess for the try command
def generate_random_guess():
    return ''.join(random.choices(COLORS, k=4))

# Function to create a single file with all test commands
def create_combined_command_file(plid, max_playtime, num_tries=4):
    with open(OUTPUT_FILE, 'w') as file:
        # Write the start command
        file.write(f"start {plid} {max_playtime}\n")
        file.write("\n")  # Blank line for readability

        # Write the try commands
        for i in range(1, num_tries + 1):
            guess = generate_random_guess()
            file.write(f"try {guess}\n")

        file.write("\n")  # Blank line for readability

        # Write the quit command
        file.write("quit\n")
        file.write("\n")  # Blank line for readability

        # Write the debug command with a known secret key
        file.write(f"debug {plid} {max_playtime} RGBY\n")

    print(f"Combined test command file created: {OUTPUT_FILE}")

# Run the function to create the test file
if __name__ == "__main__":
    create_combined_command_file(PLID, MAX_PLAYTIME, num_tries=4)
