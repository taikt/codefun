import matplotlib.pyplot as plt

cpu_list = []
mem_list = []

for i in range (1, 201, 1):
    file_name = "test_" + str(i) + ".txt"
    with open(file_name) as f:
        content = f.readlines()
        line = list(content[-2].split())
        cpu_usage = line[-4]
        mem_usage = line[-3]
        cpu_list.append(cpu_usage)
        mem_list.append(mem_usage)
#print(cpu_list)

print(mem_list)

plt.ylabel("Mem Usage (%)")

plt.xlabel("Number of Transfer")
plt.plot(mem_list)
plt.grid()
plt.show()