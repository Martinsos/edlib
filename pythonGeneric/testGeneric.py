import edlib

#query = [1,2,3,4,5,5,'c']
#target = [1,2,3,4,4,'b']

query = [i for i in range(500)]
target = [i for i in range(200, 600)]

result = edlib.align(query, target, task ='path',additionalEqualities=[('c',10),(10,'b')])
alignment = edlib.getNiceAlignment(result, query, target)

print("query = ", query)
print("target = ", target)
print("alignment = ")
print(alignment)
