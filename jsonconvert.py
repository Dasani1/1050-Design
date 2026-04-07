import json 

file = "data.txt"

array = []
with open(file,"r") as f:
    for line in f:
        line = line.strip() #Clear whitespace
        if line: #idk bro
            array.append(json.loads(line)) #txt file is already formatted into json so you don't need much

with open("data.json", "w") as f:
    json.dump(array,f,indent=2) #It just looks better, indents code to make it legible


input("Json File should be created by now! Press Enter to Leave ") #Just to let the user see that the file actually ran