'''
This is for timing calculation
The algorithm is as follows
look for cell( then look for pin( then look for related_pin 
find cell_rise/cell_rise and then look for values . read the 4th line after values
'''
outfile=open('cell.rise','w+')               
gatelist=[]
readcell=False
readpin=False
readrelatedpin=False
readrise=False
readvalues=False
count=0
val=0
with open('umcp.lib') as infile:
    for line in infile:
        if 'cell(' in line:
            gatelist.append(line)
infile.close()

rise_list={}
pin={}
with open('umcp.lib') as infile:
    for line in infile:
        if 'cell(' in line:
            pincount=0
            print line.split()
            cell=line.split()[0]
            readcell=True
            rise_list[cell]=[]
        if(readcell):
            if 'pin(' in line:
                readpin=True
        if(readpin):
            if('related_pin' in line):
                readrelatedpin=True
                temp=line.split(':')[1].split('"')[1]
                pin[pincount]=[]
        if(readrelatedpin):
            if('sdf_cond' in line):
                readrelatedpin=False
        if(readrelatedpin):
            if('cell_rise' in line):
                readrise=True
        if(readrise):
            if('values' in line):
                readvalues=True
        if(readvalues):
            count=count+1
        if(count==4):
            delays=line.split('"')[1].split(',')
            for f in range(0,len(delays)):

                pin[pincount].append(delays[f])
            pincount=pincount+1
            readrelatedpin=False
            readrise=False
            readvalues=False
            count=0
       
        if('cellend' in line):
            newdelay=[]  
            for x in range(0,7):
                val=0
                for t in range(0,pincount):
#                    print pin[t][x]
                    val=float(pin[t][x])+val
                val=val/pincount
                rise_list[cell].append(val)
#            rise_list[cell].append(newdelay)
            val=0
for keys in rise_list.keys():
    print rise_list[keys]

#for keys in pin.keys():
 #    print pin[keys]


                    
            
            
            
for keys in rise_list.keys():
    outfile.write(str(keys.split('(')[1].split(')')[0])+'\t')
    for i in range(0,len(rise_list[keys])):
        outfile.write(str(rise_list[keys][i])+'\t')
    outfile.write('\n')

