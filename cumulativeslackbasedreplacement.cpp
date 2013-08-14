#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <string>
#include <queue>
#include <math.h>
#include <sys/time.h>
#include <vector>
#include <set>
#include <cfloat>
#include <utility>
#include <map>
#include <algorithm>
#define MIN_SLACK 1
#define NUM_VT 3
using namespace std;
    struct VertexProperty
{
    vector<int> successors;
    vector<int> predecessors;
    string type;
    double leakage;
    bool state;
    double slack;
    double delay;
    double arrival;
    double cumulativeslack;
    double metric;
    int level;
    int num_replacements;
   
};
class custom_bfs_visitor:public boost::default_bfs_visitor
{
    public:
        template<typename Vertex,typename Graph>
           void discover_vertex(Vertex u,const Graph &g) 
            {
                ofstream myfile;
                myfile.open("bfs.txt",ios::out|ios::app);
                myfile<<u<<"\n";
                myfile.close();
            }
};
bool mysortfunction(const pair<int,double> &i,const pair<int,double> &j)
{
    return i.second>j.second;
}
typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::bidirectionalS,VertexProperty> Graph;
int gate_level(int element,Graph &g)
{
    vector<int>::iterator intit;
    int i,l,lmax=-1;
    int tempnode;
    if(g[element].level==-1)
    {
        lmax=-1;
        for(intit=g[element].predecessors.begin();intit<g[element].predecessors.end();intit++)
        {
            tempnode=*intit;
            l=gate_level(tempnode,g);
            if(l>lmax)
                lmax=l;
        }
        g[element].level=lmax+1;
        return(g[element].level);
    }
    else
        return (g[element].level);
}
    
double arrival_time(int levels,int size,Graph &g)
{
        vector<int>::iterator intit;

    int num=size;
    double arrival,slack,required;
    arrival=0;
    double delay=0;
    

    int tempnode;        
    g[num-1].arrival=0;
    for(int i=0;i<=levels;i++)
    {
        for(int f=0;f<num;f++)
        {
            if(g[f].level==i && i==0)
                g[f].arrival=0;       
            if(g[f].level==i && i!=0)
            {
                for(intit=g[f].predecessors.begin();intit<g[f].predecessors.end();intit++)
                {
                    tempnode=*intit;
                    if(g[tempnode].arrival+g[f].delay>arrival)
                        arrival=g[tempnode].arrival+g[f].delay;
                }
                g[f].arrival=arrival;
                arrival=0;
            }
        }
    }
    delay=0;// delay is initialized to zero
    for(int f=0;f<num;f++)
    {
        if(g[f].type=="PO" && g[f].arrival>delay)
            delay=g[f].arrival;
    }//delay = delay of the circuit
            

        
//        cout<<"verifying"<<"\n";
//        for(int f=0;f<num;f++)
//            cout <<f<<"\t"<<g[f].type<<"\t"<<g[f].arrival<<"\n";
//        cout<<"delay"<<delay<<"\n";
        return delay;
}
void undo_arrival_time(int levels,int size,Graph &g)
{
        vector<int>::iterator intit;

    int num=size;
    double arrival,slack,required;
    arrival=0;
    double delay=0;
    

    int tempnode;        
    for(int i=0;i<=levels;i++)
    {
        for(int f=0;f<num;f++)
        {
            if(g[f].level==i && i==0)
                g[f].arrival=0;       
            if(g[f].level==i && i!=0)
            {
                for(intit=g[f].predecessors.begin();intit<g[f].predecessors.end();intit++)
                {
                    tempnode=*intit;
                   if(g[tempnode].arrival+g[f].delay>arrival)
                        arrival=g[tempnode].arrival+g[f].delay;
                }
                g[f].arrival=arrival;
                arrival=0;
            }
        }
    }
    delay=0;// delay is initialized to zero
    for(int f=0;f<num;f++)
    {
        if(g[f].type=="PO" && g[f].arrival>delay)
            delay=g[f].arrival;
    }
//        
//        cout<<"verifying"<<"\n";
//        for(int f=0;f<num;f++)
//            cout <<f<<"\t"<<g[f].type<<"\t"<<g[f].arrival<<"\n";
//        cout<<"delay"<<delay<<"\n";
        //return delay;
}    

   double required_time(double delay,int levels,int size,Graph &g)
{
        vector<int>::iterator intit;
    int num=size;
    double arrival,slack,required;
        required=DBL_MAX;
        int tempnode,markednode;
        double tempcumulative=0;
        double cumulativeslack=0;
        for(int i=levels;i>=0;i--)
        {
         for(int f=num-1;f>=0;f--)
         {
             if(g[f].level==i && i==levels)
             {
                 
                 g[f].slack=delay-g[f].arrival;
             }
             else if(g[f].level==i && i!=levels && i!=0 && g[f].type=="PO")
             {
                 g[f].slack=delay-g[f].arrival;
             }
             
             else if(g[f].level==i && i!=levels && i!=0 && g[f].type!="PO")
             {
                 for(intit=g[f].successors.begin();intit<g[f].successors.end();intit++)
                {
                    tempnode=*intit;
                    if((g[tempnode].slack+g[tempnode].arrival)-g[tempnode].delay<required)
                        required=(g[tempnode].arrival+g[tempnode].slack)-g[tempnode].delay;
                    
                }

                g[f].slack=required-g[f].arrival;
                required=DBL_MAX;
             }
         }
        }

             for(int f=0;f<num;f++)
             {
                if(g[f].type!="PI" || g[f].type!="PO")
                {
                 for(intit=g[f].successors.begin();intit<g[f].successors.end();intit++)
                 {
                     tempnode=*intit;
                     cumulativeslack=g[tempnode].slack+cumulativeslack;
                     if(cumulativeslack>tempcumulative && g[tempnode].num_replacements<NUM_VT)
                     {
                         markednode=tempnode;
                         tempcumulative=cumulativeslack;

                     }
                 }
                 g[f].cumulativeslack=cumulativeslack;
                 cumulativeslack=0;
                 g[f].metric=g[f].slack*g[f].cumulativeslack;
                
                required=DBL_MAX;
                 }
             }
         
        
        cout<<markednode<<" is the marked node"<<endl;
        return markednode;
}

int main(int argc,char *argv[])
{

    string line;
    int num,num1;
    if(argc!=5)
        cout<<"usage:./a.out hgrfile typefile timing leakage "<<endl;
    else
    {
        ifstream hgrfile(argv[1]);
        if(hgrfile.is_open())
        {
            getline(hgrfile,line);
            cout<<line<<endl;
            std::istringstream ss(line);
            ss >> num1;
            num1++;
            cout<<"total number of nodes in graph="<<num1<<endl;
            Graph g(num1); 
            cout<<"created graph"<<endl;
            int count1=0;
            while(hgrfile.good())
            {
               // string line;
                getline(hgrfile,line);
                cout<<line<<endl;
                if(!line.empty())
                {
                    istringstream ss(line);
                    while(ss >> num)
                    {
                        boost::add_edge(count1,num,g);
            //            cout<<num<<"\t";
                    }
                }
                count1++;
//                cout<<count1<<endl;
            }
            num=num1-1;
            typedef boost::graph_traits<Graph>::vertex_iterator VItr;
            VItr vitr,vend;
            boost::tie(vitr,vend)=boost::vertices(g);
            ifstream propertyfile(argv[2]);
            cout<<"reading types"<<endl;
            for(;vitr!=vend;++vitr) // we assign the gates their types pi po or cell
            {
                if(propertyfile.is_open())
                    getline(propertyfile,line);
                g[*vitr].type=line;
                cout<<*vitr<<" "<<g[*vitr].type<<endl;
            }
            boost::tie(vitr,vend)=boost::vertices(g);
            int j,k;
            boost::tie(vitr,vend)=boost::vertices(g);
            VItr vitr1,vend1;
            boost::tie(vitr1,vend1)=boost::vertices(g);
            cout<<"identifying fanins/ fanouts"<<endl;
            for(int i=0;i<num;i++)
            {
                vector<int>successors;
                for(int j=0;j<num;j++)
                    if(boost::edge(i,j,g).second)
                        successors.push_back(j);
                g[i].successors=successors;
            }
//            for(int i=0;i<num;i++)
//            {
//            //    cout<<i<<"\n";
//                for(int j=0;j<num;j++)
//
//                    if(boost::edge(i,j,g).second)
//              //          cout<<j<<"\t";
//               // cout<<"\n";
//            }
            for(int i=0;i<num;i++)
            {
                vector<int>predecessors;
                for(int j=0;j<num;j++)
                    if(boost::edge(j,i,g).second )
                        predecessors.push_back(j);
                g[i].predecessors=predecessors;
            }
//            vector<int>::iterator vecit;
//            for(int i=0;i<num;i++)
//            {
//                cout<<i<<"\n";
//                for(vecit=g[i].successors.begin();vecit<g[i].successors.end();vecit++)
//                    cout<<*vecit<<"\t";
//            }
//            ifstream pifile(argv[3]);
//            int stateval;
            int pi;
            map<string,vector<double>> leakagemap;
            map<string,vector<double>> timingmap;
            string tempword;
            double tempnum;
            ifstream leakagefile(argv[4]);
            while (leakagefile.good())
            {
                getline(leakagefile,line);
                stringstream ss(line);
                ss>>tempword;
                while(ss>>tempnum)
                {

                    leakagemap[tempword].push_back(tempnum);
                }

            }
            ifstream timingfile(argv[3]);
            while (timingfile.good())
            {
                getline(timingfile,line);
                stringstream ss(line);
                ss>>tempword;
                while(ss>>tempnum)
                    timingmap[tempword].push_back(tempnum);
            }

            for(int t=0;t<num;t++)//iterate through all the cells
            {
                if(g[t].type=="PI")//if no fanin then it is pi
                {
                    boost::add_edge(num,t,g);
                        g[t].leakage=0;
                }
                else if(g[t].predecessors.size()!=0 && g[t].successors.size()!=0)
                {
                   
                   g[t].leakage=leakagemap[g[t].type][0];
                }
                else if(g[t].successors.size()==0)
                    g[t].leakage=0;
            }
            cout<<"Leakage assignment for nodes is completed!"<<endl;
             double temp_delay;
             
             for(int t=0;t<num;t++)
             {
                 if(g[t].predecessors.size()!=0 && g[t].successors.size()!=0)
                 {
                    g[t].delay=timingmap[g[t].type][0];


                 }
                 else if(g[t].predecessors.size()==0||g[t].successors.size()==0)
                 {
                     g[t].delay=0;
                 }
             }  
             cout<<"displaying leakage and delay values"<<endl;;
            for(int i=0;i<num;i++)
            {
                cout<<i<<"\t"<<g[i].leakage<<"\t"<<g[i].delay<<"\n";
                
            }
            cout<<"\n";
        cout<<"num :"<<num<<"\n";
        for (int i=0;i<=num;i++)
            g[i].level=-1;
        cout<<"print dummy cells\n";
        for(int i=0;i<=num;i++)
            if(g[i].predecessors.size()==0 && g[i].type!="PI")
                cout<<i<<" "<<g[i].type<<"\n";
        cout<<"\n";
 cout<<"num :"<<num<<"\n";
        cout<<"done with timing"<<endl;
        int *visited=(int*)malloc((num1)*sizeof(int));
        int *visit=(int*)malloc((num1)*sizeof(int));
        for(int i=0;i<num;i++)
        {
            visited[i]=0;
            visit[i]=0;
        }



        queue<int> nodequeue;
        vector<int> bfsarray;
        vector<int>::iterator intit1;
        int front,rear;
        int v=num1-1;
        cout<<num1<<"\n"; 
        bfsarray.push_back(v);
        //v=bfsarray[0];

        visited[v]=1;
        k=0;
        pi=0;
        custom_bfs_visitor vis;
        breadth_first_search(g,vertex(num,g),visitor(vis));
        vector<int> povector;
        for(int i=0;i<=num;i++)
        {
            
            if(g[i].type=="PO")
            {
                cout<<"FOR PO "<<i<<" :fanins are"<<"\t";
                povector.push_back(i);
                for(intit1=g[i].predecessors.begin();intit1<g[i].predecessors.end();intit1++)
                {
                    cout<<*intit1<<"\t";
                    
                }
                cout<<"fanout size "<<g[i].successors.size();
                cout<<"\n";
            }
        }
        int markednode;
        cout<<"did bfs"<<endl;
        int tempo;
         int templevel=-1;

        for(int i=0;i<povector.size();i++)
        {
            tempo=povector[i];
            g[tempo].level=gate_level(tempo,g);
            if(g[tempo].level>templevel)
                templevel=g[tempo].level;
            

        }
         cout<<"max level is "<<templevel;      
         for(int i=0;i<num1;i++)
             cout<<i<<" "<<g[i].level<<"\n";
         vector<double> tempvect;
        vector<int>::iterator intit;
        double cumulativeslack;
        vector<bool>::iterator boolit;
        double arrival,slack,required;
        arrival=0;
        double delay=0;
        delay=arrival_time(templevel,num1,g);
        markednode=required_time(delay,templevel,num1,g);
        vector<pair<int,double>> slackset;
        cout<<"delay="<<delay<<endl;
//        double tempdouble=static_timing(templevel,num1,g);
        double tempdelay;
        int t=0;
        int count=0,undocount=0;
        int a=1;
        for(int i=0;i<num1;i++)
            g[t].num_replacements=0;
        for(int i=0;i<num1;i++)
            cout<<i<<" "<<" "<<g[i].arrival<<" "<<g[i].slack<<" "<<g[i].cumulativeslack<<"\n";
        cout<<"MARKED NODE="<<markednode<<", CSLACK="<<g[markednode].cumulativeslack<<", SLACK="<<g[markednode].slack<<endl;
        cumulativeslack= g[markednode].cumulativeslack;
        t=markednode;
        while(cumulativeslack>MIN_SLACK){
            
                
                    cout<<"current acumulated slack="<<g[t].cumulativeslack<<endl;
                    tempvect=leakagemap[g[t].type];
                    for(int i=0;i<tempvect.size();i++)
                        if(tempvect[i]==g[t].leakage && i+1<=tempvect.size())
                            a=tempvect[i+1];
                     g[t].leakage=a;
                    tempvect=timingmap[g[t].type];
                    for(int i=0;i<tempvect.size();i++)
                        if(tempvect[i]==g[t].delay && i+1<=tempvect.size())
                            a=tempvect[i+1];
                    g[t].delay=a;
                    g[t].num_replacements=g[t].num_replacements+1;
                    cout<<"current node="<<t<<endl<<"new node leakage="<<g[t].leakage<<", new node delay="<<g[t].delay<<endl;
                    tempdelay=arrival_time(templevel,num1,g);
                    cout<<"new circuit delay=" <<tempdelay<<endl;
                    cout<<"delay computation is over!"<<endl;

                    if(tempdelay>delay)
                    {
                       tempvect=leakagemap[g[t].type];
                        for(int i=0;i<tempvect.size();i++)
                            if(tempvect[i]==g[t].leakage)
                                a=tempvect[i-1];
                        g[t].leakage=a;
                        tempvect=timingmap[g[t].type];
                        for(int i=0;i<tempvect.size();i++)
                            if(tempvect[i]==g[t].delay)
                                a=tempvect[i-1];
                        g[t].delay=a;
//                       g[t].leakage=leakagemap[g[t].type][0];
//                        g[t].delay=timingmap[g[t].type][0];
                      undo_arrival_time(templevel,num1,g);
                      cout<<"undoing gate replacement"<<endl;
                      undocount++;
                        cout<<"number of undid gate replacements="<<undocount<<"\n";
                    }
                
            else
                markednode=required_time(delay,templevel,num1,g);
            cumulativeslack=g[markednode].cumulativeslack;
            t=markednode;
           cout<<"new node selected="<<t<<endl; 
        count++;
        cout<<"total gate replacement attempts so far="<<count<<"\n";
        }
        cout<<"number of undid replacements="<<undocount<<", number of total replacements="<<count<<"\n";

                 string filename="out.txt";
                ofstream fout(filename.c_str());
                boost::write_graphviz(fout,g);
                       



    }//for hgrfile


    }//closing braces for argument else

    return 0;
}//closing braces for int main()

