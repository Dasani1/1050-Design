import json 

file = "DATA 1.txt"

array = []
with open(file,"r") as f:
    for line in f:
        line = line.strip() #Clear whitespace
        if line: #idk bro
            array.append(json.loads(line)) #ig cause it's already formatted I don't need to do much

with open("data.json", "w") as f:
    json.dump(array,f,indent=2) #It just looks better


input("Json File should be created by now! Press Enter to Leave ")