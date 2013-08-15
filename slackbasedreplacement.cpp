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
#define MIN_SLACK 0
#define NUM_VT 3
using namespace std;
    struct VertexProperty
{
    vector<int> successors;
    vector<int> predecessors;
    string type;
    double leakage;
    double capacitance;
    vector<double> slack;
    vector<double> delay;
    vector<double> arrival;
    double cumulativeslack;
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
double interpolate(double xleft,double xright,double yleft,double yright,double xval)
{
    double y=yleft+(((xval-xleft)*(yright-yleft))/(xright-xleft));
    return y;
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

//double arrival_time(int levels,int size,Graph &g)
//{
//        vector<int>::iterator intit;
//
//    int num=size;
//    double arrival,slack,required;
//    arrival=0;
//    double delay=0;
//    
//
//    int tempnode;        
//    g[num-1].arrival=0;
//    for(int i=0;i<=levels;i++)
//    {
//        for(int f=0;f<num;f++)
//        {
//            if(g[f].level==i && i==0)
//                g[f].arrival=0;       
//            if(g[f].level==i && i!=0)
//            {
//                for(intit=g[f].predecessors.begin();intit<g[f].predecessors.end();intit++)
//                {
//                    tempnode=*intit;
//                    if(g[tempnode].arrival+g[f].delay>arrival)
//                        arrival=g[tempnode].arrival+g[f].delay;
//                }
//                g[f].arrival=arrival;
//                arrival=0;
//            }
//        }
//    }
//    delay=0;// delay is initialized to zero
//    for(int f=0;f<num;f++)
//    {
//        if(g[f].type=="PO" && g[f].arrival>delay)
//            delay=g[f].arrival;
//    }//delay = delay of the circuit
//            
//        return delay;
//}
//void undo_arrival_time(int levels,int size,Graph &g)
//{
//        vector<int>::iterator intit;
//
//    int num=size;
//    double arrival,slack,required;
//    arrival=0;
//    double delay=0;
//    
//
//    int tempnode;        
//    for(int i=0;i<=levels;i++)
//    {
//        for(int f=0;f<num;f++)
//        {
//            if(g[f].level==i && i==0)
//                g[f].arrival=0;       
//            if(g[f].level==i && i!=0)
//            {
//                for(intit=g[f].predecessors.begin();intit<g[f].predecessors.end();intit++)
//                {
//                    tempnode=*intit;
//                   if(g[tempnode].arrival+g[f].delay>arrival)
//                        arrival=g[tempnode].arrival+g[f].delay;
//                }
//                g[f].arrival=arrival;
//                arrival=0;
//            }
//        }
//    }
//    delay=0;// delay is initialized to zero
//    for(int f=0;f<num;f++)
//    {
//        if(g[f].type=="PO" && g[f].arrival>delay)
//            delay=g[f].arrival;
//    }
//}    
//vector<pair<int,double>> required_time(double delay,int levels,int size,Graph &g)
//{
//        vector<int>::iterator intit;
//    int num=size;
//    double arrival,slack,required;
//        required=DBL_MAX;
//        int tempnode,markednode=size-1;
//        double maxslack=DBL_MIN;
//        double cumulativeslack=0;
//        for(int i=levels;i>=0;i--)
//        {
//         for(int f=num-1;f>=0;f--)
//         {
//             if(g[f].level==i && i==levels)
//             {
//                 
//                 g[f].slack=delay-g[f].arrival;
//             }
//             else if(g[f].level==i && i!=levels && i!=0 && g[f].type=="PO")
//             {
//                 g[f].slack=delay-g[f].arrival;
//             }
//             
//             else if(g[f].level==i && i!=levels && i!=0 && g[f].type!="PO")
//             {
//                 for(intit=g[f].successors.begin();intit<g[f].successors.end();intit++)
//                {
//                    tempnode=*intit;
//                    if((g[tempnode].slack+g[tempnode].arrival)-g[tempnode].delay<required)
//                        required=(g[tempnode].arrival+g[tempnode].slack)-g[tempnode].delay;
//                    
//                }
//
//                g[f].slack=required-g[f].arrival;
//                required=DBL_MAX;
//             }
//         }
//
//        }
////        vector<int> nodevector;
//                     vector<pair<int,double>> slackset;
//
//        slack=0;
//             for(int f=0;f<num;f++)
//             {
//                if(g[f].type!="PI" && g[f].type!="PO")
//                {
//
//        //             cumulativeslack=g[tempnode].slack+cumulativeslack;
//                     slack=g[f].slack;
//                     if(slack>=maxslack && g[f].num_replacements<NUM_VT )
//                     {
//                         markednode=tempnode;
//                         maxslack=slack;
//                        slackset.push_back(std::make_pair(f,g[f].slack));
//                        cout<<"TYPE inside function="<<g[f].type<<endl;
//
////                         nodevector.push_back(markednode);
//
//                     }
//                 }
////                 g[f].cumulativeslack=cumulativeslack;
//                 slack=0;
//                 //g[f].metric=g[f].slack*g[f].cumulativeslack;
//                
//                required=DBL_MAX;
//                 
//             }
//        
//        //     for(int i=0;i<num;i++)
//        sort(slackset.begin(),slackset.end(),mysortfunction);
//        ofstream myfile;
//        myfile.open("slackfile.txt",ios::out|ios::app);
//        myfile<<"the nodes in the decreasing order of slack are\n";
//        for(int i=0;i<slackset.size();i++)
//            myfile<<slackset[i].first<<" ";
//        myfile<<"\n";
//        myfile.close();
//       // slackset.clear();
//        cout<<markednode<<" is the marked node"<<endl;
//        return slackset;
//}
//void second_required_time(double delay,int levels,int size,Graph &g)
//{
//        vector<int>::iterator intit;
//    int num=size;
//    double arrival,slack,required;
//        required=DBL_MAX;
//        int tempnode,markednode=size-1;
//        double maxslack=DBL_MIN;
//        double cumulativeslack=0;
//        for(int i=levels;i>=0;i--)
//        {
//         for(int f=num-1;f>=0;f--)
//         {
//             if(g[f].level==i && i==levels)
//             {
//                 
//                 g[f].slack=delay-g[f].arrival;
//             }
//             else if(g[f].level==i && i!=levels && i!=0 && g[f].type=="PO")
//             {
//                 g[f].slack=delay-g[f].arrival;
//             }
//             
//             else if(g[f].level==i && i!=levels && i!=0 && g[f].type!="PO")
//             {
//                 for(intit=g[f].successors.begin();intit<g[f].successors.end();intit++)
//                {
//                    tempnode=*intit;
//                    if((g[tempnode].slack+g[tempnode].arrival)-g[tempnode].delay<required)
//                        required=(g[tempnode].arrival+g[tempnode].slack)-g[tempnode].delay;
//                    
//                }
//
//                g[f].slack=required-g[f].arrival;
//                required=DBL_MAX;
//             }
//         }
//        }
//}

int main(int argc,char *argv[])
{

    string line;
    int num,num1;
    if(argc!=5)
        cout<<"usage:./a.out hgrfile typefile timing leakage "<<endl;
    else
    {
        Graph g; 

        ifstream hgrfile(argv[1]);
        if(hgrfile.is_open())
        {
            getline(hgrfile,line);
            cout<<line<<endl;
            std::istringstream ss(line);
            ss >> num1;
            num1++;
            cout<<"total number of nodes in graph="<<num1<<endl;
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
            for(int i=0;i<num;i++)
            {
                vector<int>predecessors;
                for(int j=0;j<num;j++)
                    if(boost::edge(j,i,g).second )
                        predecessors.push_back(j);
                g[i].predecessors=predecessors;
            }
            int pi;
            map<string,vector<double>> leakagemap;
            map<string,vector<double>> risemap;
            map<string,vector<double>> fallmap;
            map<string,double> capacitancemap;
            string tempword;
            double tempnum;
            ifstream leakagefile(argv[3]);
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
            // additions made on 15-08-2013
            vector<int>::iterator vecit;
            ifstream risefile(argv[4]);
            while (risefile.good())
            {
                getline(risefile,line);
                stringstream ss(line);
                ss>>tempword;
                while(ss>>tempnum)
                    risemap[tempword].push_back(tempnum);
            }
            ifstream fallfile(argv[5]);
            while(fallfile.good())
            {
                getline(fallfile,line);
                stringstream ss(line);
                ss>>tempword;
                while(ss>>tempnum)
                    fallmap[tempword].push_back(tempnum);
            }
            ifstream capfile(argv[6]);
            while(capfile.good())
            {
                getline(capfile,line);
                stringstream ss(line);
                ss>>tempword;
                while(ss>>tempnum)
                    capacitancemap[tempword]=tempnum;
            }
            int tempnode;
            for (int t=0;t<num;t++)
            {
                if(g[t].type!="PI" && g[t].type!="PO")
                {
                    double tempcap=0;
                    for( vecit=g[t].successors.begin();vecit!=g[t].successors.end();vecit++)
                    {
                        tempnode=*vecit;
                        tempcap=capacitancemap[g[tempnode].type]+tempcap;
                    }
                    


                    g[t].capacitance=tempcap;
                }
                else
                    g[t].capacitance=0;
            }
            
//            delay_function(g);
        vector<double> load_capacitance;//initialize with load_capacitance_values
    

       double tempdelay;
       int x1,x2;
       double xleft,xright,yleft,yright,xval;
       for (int t=0;t<num;t++)
       {
           if(g[t].type!="PI" && g[t].type!="PO")
           {
              for(int i=0;i<load_capacitance.size();i++)
              {
                  tempdelay=load_capacitance[i];
                  if(tempdelay>=g[t].capacitance)
                  {
                          x1=i-1;
                          x2=i;
                          break; 
                      
                      
                  }
                  else if (i==load_capacitance.size()-1 && load_capacitance[i] < g[t].capacitance)
                  {
                      x1=i-1;
                      x2=i;
                      break;
                  }
    
              }
              xleft=load_capacitance[x1];
              yleft=risemap[g[t].type][x1];
              xright=load_capacitance[x2];
              yright=risemap[g[t].type][x2];
              xval=g[t].capacitance;
              g[t].delay.push_back(interpolate(xleft,xright,yleft,yright,xval));
              yleft=fallmap[g[t].type][x1];
              yright=fallmap[g[t].type][x2];
              g[t].delay.push_back(interpolate(xleft,xright,yleft,yright,xval));
    
    
           }
               
        }
                // end of changes made on 15-08-2013

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
             
//             for(int t=0;t<num;t++)
//             {
//                 if(g[t].predecessors.size()!=0 && g[t].successors.size()!=0)
//                 {
//                    g[t].delay=timingmap[g[t].type][0];
//
//
//                 }
//                 else if(g[t].predecessors.size()==0||g[t].successors.size()==0)
//                 {
//                     g[t].delay=0;
//                 }
//             }  
//             cout<<"displaying leakage and delay values"<<endl;;
//            for(int i=0;i<num;i++)
//            {
//                cout<<i<<"\t"<<g[i].leakage<<"\t"<<g[i].delay<<"\n";
//                
          //  }
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
//        vector<int>::iterator intit;
////        double cumulativeslack;
//        vector<bool>::iterator boolit;
//        double arrival,slack,required;
//        arrival=0;
//        double delay=0;
//        delay=arrival_time(templevel,num1,g);
//        vector<pair<int,double>> slackset=required_time(delay,templevel,num1,g);
//        cout<<"delay="<<delay<<endl;
////        double tempdouble=static_timing(templevel,num1,g);
//        double tempdelay;
//        int t=0;
//        int count=0,undocount=0;
//        int a=1;
//        for(int i=0;i<slackset.size();i++)
//        {
//          
//
//           t=slackset[i].first;
//            cout<<"TYPE="<<g[t].type<<endl;          
//              tempvect=leakagemap[g[t].type];
//    
//               g[t].leakage=leakagemap[g[t].type][1];
//               tempvect=timingmap[g[t].type];
//               g[t].delay=tempvect[1];
//           
//        }
//        cout <<"total nodes replaced in one go="<<slackset.size()<<endl;
//        //double tempdelay;
//        tempdelay=arrival_time(templevel,num1,g);
//        cout<<"delay after replacement="<<tempdelay<<"\n";
//        second_required_time(tempdelay,templevel,num1,g);
//        for(int i=0;i<num1;i++)
//            g[t].num_replacements=0;
         //        string filename="out.txt";
            //    ofstream fout(filename.c_str());
             //   boost::write_graphviz(fout,g);
                       



//    }//for hgrfile


    }//closing braces for argument else

    return 0;
}//closing braces for int main()

