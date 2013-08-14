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
    if(g[element].level=-1)
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
    
double static_timing(vector<int> bfsarray,Graph &g)
{
        vector<int>::iterator intit;

    int num=bfsarray.size();
    double arrival,slack,required;
    arrival=DBL_MIN;
    double delay=0;
        for(int f=0;f<num;f++)
        {
            int u=bfsarray[f];

            
            if(g[u].type!="PI" && g[u].type!="PO")
            {
                int tempnode;
                for(intit=g[u].predecessors.begin();intit<g[u].predecessors.end();intit++)
                {
                    tempnode=*intit;
                                        if(g[tempnode].arrival+g[tempnode].delay>arrival)
                        arrival=g[tempnode].arrival+g[tempnode].delay;
                }
                g[u].arrival=arrival;
                arrival=DBL_MIN;
                               }
            if(g[u].type=="PI")
                g[u].arrival=0;
            }
        
        for(int f=0;f<bfsarray.size();f++)
        {
            int u=bfsarray[f];
        int tempnode;
            if(g[u].type=="PO")
            {
                for(intit=g[u].predecessors.begin();intit<g[u].predecessors.end();intit++)
                {
                    tempnode=*intit;
                    if(g[tempnode].arrival+g[tempnode].delay>arrival)
                        arrival=g[tempnode].arrival+g[tempnode].delay;
                }
                g[u].arrival=arrival;
                if(arrival>delay)
                    delay=arrival;
            }
        }
        cout<<"verifying"<<"\n";
        for(int f=0;f<num;f++)
            cout <<bfsarray[f]<<"\t"<<g[bfsarray[f]].type<<"\t"<<g[bfsarray[f]].arrival<<"\n";
        cout<<"delay"<<delay<<"\n";
        return delay;
}
void undo(vector<int> bfsarray,Graph &g)
{
        vector<int>::iterator intit;

    int num=bfsarray.size();
    double arrival,slack,required;
     for(int f=0;f<=num;f++)
      g[bfsarray[f]].arrival=0;

    arrival=DBL_MIN;
    double delay=0;
        for(int f=0;f<=num;f++)
        {
            int u=bfsarray[f];
            if(g[u].predecessors.size()!=0 && g[u].successors.size()!=0)
            {
                vector<bool> temp;
                int tempnode;
                bool tempstate;
                for(intit=g[u].predecessors.begin();intit<g[u].predecessors.end();intit++)
                {
                    tempnode=*intit;
                    temp.push_back(g[tempnode].state);
                    if(g[tempnode].arrival+g[tempnode].delay>arrival)
                        arrival=g[tempnode].arrival+g[tempnode].delay;
                }
                g[u].arrival=arrival;
                arrival=DBL_MIN;
                               }
            if(g[u].predecessors.size()==0)
                g[u].arrival=0;
        }
        for(int f=0;f<=num;f++)
        {
            int u=bfsarray[f];
            int tempnode;
            if(g[u].successors.size()==0)
            {
                for(intit=g[u].predecessors.begin();intit<g[u].predecessors.end();intit++)
                {
                    tempnode=*intit;
                    if(g[tempnode].arrival+g[tempnode].delay>arrival)
                        arrival=g[tempnode].arrival+g[tempnode].delay;
                }
                g[u].arrival=arrival;
                if(arrival>delay)
                    delay=arrival;
            }
        }
        cout<<"delay after undoing is "<<delay<<endl;
     
}
   void required_time(double delay,vector<int> bfsarray,Graph &g)
{
        vector<int>::iterator intit;

    int num=bfsarray.size();
    double arrival,slack,required;

        required=DBL_MAX;
        int tempnode;
        double cumulativeslack=0;
         for(int f=num-1;f>=0;f--)
         {
            int u=bfsarray[f];
             if(g[u].successors.size()==0)
             {
                 
                 g[u].slack=delay-g[u].arrival;
             }
             if(g[u].successors.size()!=0)
             {
                 for(intit=g[u].successors.begin();intit<g[u].successors.end();intit++)
                {
                    tempnode=*intit;
                    if((g[tempnode].slack+g[tempnode].arrival)-g[tempnode].delay<required)
                        required=(g[tempnode].arrival+g[tempnode].slack)-g[tempnode].delay;
                }
                 for(intit=g[u].successors.begin();intit<g[u].successors.end();intit++)
                 {
                     tempnode=*intit;
                     cumulativeslack=g[tempnode].slack+cumulativeslack;
                 }
                 g[u].cumulativeslack=cumulativeslack;
                 cumulativeslack=0;
                 g[u].metric=g[u].slack*g[u].cumulativeslack;
                
                g[u].slack=required-g[u].arrival;
                required=DBL_MAX;
             }
         }
               

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
            cout<<"num1 is "<<num1<<endl;
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
            cout<<"doing fanins or fanouts"<<endl;
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
            string templine;
            double tempnum;
            ifstream leakagefile(argv[4]);
            while (leakagefile.good())
            {
                getline(leakagefile,line);
                stringstream ss(line);
                ss>>templine;
                while(ss>>tempnum)
                {

                    leakagemap[templine].push_back(tempnum);
                }

            }
            ifstream timingfile(argv[3]);
            while (timingfile.good())
            {
                getline(timingfile,line);
                stringstream ss(line);
                ss>>templine;
                while(ss>>tempnum)
                    timingmap[templine].push_back(tempnum);
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
            cout<<"done with leakage"<<endl;
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
             cout<<"displaying values"<<endl;;
            for(int i=0;i<num;i++)
            {
                cout<<i<<"\t"<<g[i].leakage<<"\t"<<g[i].delay<<"\n";
                
            }
            cout<<"\n";
        cout<<"num :"<<num<<"\n";
        for (int i=0;i<=num;i++)
            g[i].level=-1;
        cout<<"print rogue cells\n";
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
 //       int tempnode,nodecount,tempnode1;
 //      while(k<num){
 //           for(int i=0;i<=num;i++)
 //           {
 //                               
 //               bool tempcheck=boost::edge(v,i,g).second;
 //                               
 //                              
 //               if(tempcheck && visited[i]!=1&&visit[i]!=1  )
 //               {
 //                   // for(intit1=g[i].predecessors.begin();intit1<g[i].predecessors.end();intit1++)
 //                   //{
 //                   //    tempnode=*intit1;
 //                   //    if(find(bfsarray.begin(),bfsarray.end(),tempnode)!=bfsarray.end())
 //                   //        nodecount++;
 //                   //}
 //                   // if(nodecount==g[i].predecessors.size())
 //                   //{

 //                       visit[i]=1;
 //                       nodequeue.push(i);
 //                       nodecount=0;
 //                  // }
 //                   // else
 //                     //   nodecount=0;


 //               }
 //               else 
 //                   nodecount=0;
 //                
 //                
 //           }
////            nodecount=0;
////            bfsarray.push_back(v);
 //           k++;
 //           if(!nodequeue.empty())
 //           {
 //               v=nodequeue.front();
 //               nodequeue.pop();
 //               visit[v]=0;
 //               visited[v]=1;
 //               bfsarray.push_back(v);
 //           }
 //       }
 //       cout<<"size of bfs array "<<bfsarray.size()<<"\n";
 //          for(intit1=bfsarray.begin();intit1<bfsarray.end();intit1++)
 //      cout<<*intit1<<"\t";

   cout<<"\n";
        cout<<"did bfs"<<endl;
        int tempo;
        for(int i=0;i<povector.size();i++)
        {
            tempo=povector[i];
            g[tempo].level=gate_level(tempo,g);

        }
        int templevel=-1;
        for(int i=0;i<bfsarray.size();i++)
        {
            tempo=bfsarray[i];
            cout<<g[tempo].level<<"\n";
            if(g[tempo].level>templevel)
                templevel=g[tempo].level;
        }

        vector<int>::iterator intit;

        vector<bool>::iterator boolit;
        double arrival,slack,required;
        arrival=0;
        double delay=0;
        delay=static_timing(bfsarray,g);
        required_time(delay,bfsarray,g);
        vector<pair<int,double>> slackset;
        cout<<"delay is"<<delay<<endl;
        double tempdouble=static_timing(bfsarray,g);

//         for(intit1=bfsarray.begin();intit1<bfsarray.end();intit1++)
//         {
//             int u=*intit1;
//             if(g[u].slack!=0 && g[u].type!="PI" && g[u].type!="PO" && u!=num1-1)
//             {
//                 slackset.push_back(std::make_pair(u,g[u].cumulativeslack));
//                 
//             }
//                         
//
//         }
//         for(int x=0;x<slackset.size();x++)
//             cout<<slackset[x].first<<" "<<slackset[x].second<<"\n";
//        //sort the cumulative stackvalues and then sort the array based on sorted cumulative stack
//        sort(slackset.begin(),slackset.end(),mysortfunction);
//        cout<<"sorted"<<endl;
//        int t,count=0;
//        double tempdelay;
//        vector<double> tempvect;
//        int undocount=0;
//        do
//        {
//            t=slackset[count].first;
//            cout<<t<<"\n";
//            tempvect=leakagemap[g[t].type];
//             g[t].leakage=tempvect[1];
//            tempvect=timingmap[g[t].type];
//            g[t].delay=tempvect[1];
//            tempdelay=static_timing(bfsarray,g);
//            cout<<"doing delay"<<endl;
//
//            if(tempdelay>delay)
//            {
//                g[t].leakage=leakagemap[g[t].type][0];
//                g[t].delay=timingmap[g[t].type][0];
//              undo(bfsarray,g);
//              cout<<"undoing"<<endl;
//              undocount++;
//            
//            }
//            else
//                required_time(delay,bfsarray,g);
//            count++;
//        }while(count<slackset.size());
//        cout<<"undid moves"<<undocount<<"totalmoves:"<<slackset.size()<<"\n";
//

    



        
            
        

         





             


                
                        
                 string filename="out.txt";
                ofstream fout(filename.c_str());
                boost::write_graphviz(fout,g);
                       



    }//for hgrfile


    }//closing braces for argument else

    return 0;
}//closing braces for int main()

