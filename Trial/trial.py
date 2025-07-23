names = ["amamda", "Ken","Ken" ]

# print(names.count("bob"))


# while names.count("bob") > 1:
#     try:
#         names.remove("bob")

#     except ValueError:
#         print("Bob has just one instance")

# print(names)


try:
    names.remove("bob")  
except ValueError:
    print("Bob is not in the list")
