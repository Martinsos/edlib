import edlib

query = [1,2,3,4,5,5,10]
target = [1,2,3,4,4,10]

result = edlib.align(query, target, task ='path')
alignment = edlib.getNiceAlignment(result, query, target)

print("query = ", query)
print("target = ", target)
print("alignment = ")
print(alignment)
