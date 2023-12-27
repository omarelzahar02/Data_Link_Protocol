from lorem_text import lorem
import random
import string
def generate_test_case():
    # Generate a random 4-bit field
    error_code = ''.join(random.choice('01') for _ in range(4))

    # Generate a meaningful sentence using lorem-text
    text = lorem.sentence()

    # Format the test case
    test_case = f"{error_code} {text}"

    return test_case

# Example: Generate 5 random test cases
for _ in range(5):
    test_case = generate_test_case()
    print(test_case)
