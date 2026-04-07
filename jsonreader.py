import json
import matplotlib.pyplot as plt


#Read a json file, then graph it.
with open('data.json') as f:
    data = json.load(f)

values = [item['lux'] for item in data] #List comprehension to extract from the Json file
times = [item['Time'] for item in data]

n = 24 #Simulate "24 hours"
timetime = []
luxlux = []

for i in range(int(len(times)/n)+1): #create an empty 2d array with the amount of time and lux
#+1 incase the divided number is like 5.99
    row = [] #Python has this weird thing where each array carries the same memory so if I append row to both arrays they carry the same data and mess up
    row2 = [] #There is definitely an easier method, but I couldn't be bothered to use ai
    timetime.append(row)    
    luxlux.append(row2)
tracker = 0 #Moves on to the next array
counter = 1 #Iterates instead of going through the list as time sometimes resets at 0 for some reason

for i in times: #This is to actually sort them in a 2d array
    if counter % n != 0:
        timetime[tracker].append(i%n)
        luxlux[tracker].append(values[i-1])
    elif counter % n == 0: 
        if i % n == 0:
            timetime[tracker].append(n)
        else:
            timetime[tracker].append(i%24)
        luxlux[tracker].append(values[i-1])
      
        tracker +=1
    counter += 1

#This code has to be the most inefficient thing I've written

for i in range(len(timetime)):
    plt.plot(timetime[i],luxlux[i]) #Plots every single thing in both 2d arrays


plt.ylabel("Lux")
plt.xlabel("Time")
plt.title("Lux over 24h")
plt.show()



