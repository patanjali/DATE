'''
This is for capacitance calculation
The algorithm is as follows
look for cell( then look for pin( then look for capacitance
read capacitance for each pin and print in the row of the corresponding gate.
'''
outfile=open('cell.cap','w+')
gatelist=[]
with open('umcp.lib') as infile:
    for line in infile:
        if 'cell(' in line:
            gatelist.append(line)
infile.close()
readcell=False
readcap=False
cap_list={}
with open('umcp.lib') as infile:
    for line in infile:
        if 'cell(' in line:
            print line.split()
            cell=line.split()[0]
            readcell=True
            cap_list[cell]=[]
        if(readcell):
            if 'pin(' in line:
                readcap=True
        if(readcap):
            if('capacitance' in line):
                a=line.split(':')[1]
                cap_list[cell].append(a)
                readcap=False

for keys in cap_list.keys():
    outfile.write(str(keys)+'\t')
    for i in range(0,len(cap_list[keys])):
        outfile.write(str(cap_list[keys][i])+'\t')
    outfile.write('\n')

    
