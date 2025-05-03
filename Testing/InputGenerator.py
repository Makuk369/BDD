import itertools
import random

def generate_random_bool_function(num_vars):
    var_names = [chr(ord('A') + i) for i in range(num_vars)]
    combinations = list(itertools.product([0, 1], repeat=num_vars))

    sop_terms = []
    truth_table = []

    for combo in combinations:
        output = random.randint(0, 1)
        truth_table.append((combo, output))
        if output == 1:
            term_parts = []
            for var, val in zip(var_names, combo):
                term_parts.append(var if val == 1 else f"!{var}")
            sop_terms.append('*'.join(term_parts))
    
    with open("Testing\\boolFuncs.txt", "w") as boolFuncFile, open("Testing\\expectedOutput.txt", "w") as expOutFile:
        if sop_terms:
            bool_func = '+'.join(sop_terms)
        else:
            bool_func = "0"  # Function always outputs 0
        
        boolFuncFile.write(f"{bool_func}\n")
        
        for combo, output in truth_table:
            input_str = ''.join(map(str, combo))
            boolFuncFile.write(f"{input_str}\n")
            expOutFile.write(f"{output}\n")
    
    print("INPUT GENERATION FINISHED!")

if __name__ == "__main__":
    n = int(input("Enter number of variables: "))
    generate_random_bool_function(n)
