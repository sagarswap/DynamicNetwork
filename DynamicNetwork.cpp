#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
using namespace std;


class Node{
    int id;
    bool state;
    std::vector<Node*> neighbours;

    public:

    Node(int identity, bool status){
        this->id = identity;
        this->state = status;
    }

    void setState(Node* node){
        this->state=node->getState();
    }

    int getId(){
        return this->id;
    }

    bool getState(){
        return this->state;
    }

    Node* getNeighbour(int id){
        for(Node* node: neighbours){
            if(node->getId()==id)
                return node;
        }
        cout<<"Neighbour ID="<<id<<" not found."<<endl;
        return nullptr;
    }

    void addNeighbour(Node* newNeighbour){
        for(Node* node: neighbours){
            if(node->getId() == newNeighbour->getId()){
                cout<<this->getId()<< " already has neighbour with ID=" <<newNeighbour->getId()<<". Ignoring duplicate addition."<<endl;
                return;
            }
        }
        neighbours.push_back(newNeighbour);
    }

    void deleteNeighbour(Node* lamb){
        for (auto it = neighbours.begin(); it != neighbours.end(); ++it) {
            if (*it == lamb) {
                neighbours.erase(it);
                break;
            }
        }
    }

    void printAllNeighbours(){
        cout<<this->getId()<<" ("<<this->getState()<<")-> ";
        for(Node* node: neighbours){
            cout<<node->getId()<<" ";
        }
        cout<<endl;
    }
    
};

class DynamicNetwork{
    public:
    int nodeCount, edgeCount, stat0, stat1;
    int epochLimit, stepCount;
    double rewiringProbability, relativeSize, startingRatio;
    std::vector<Node*> nodeList;
    std::string inputFileName, outputFileName;
    long rew=0, con=0;
    bool complexContagion;

    DynamicNetwork(std::string infname, double rewire, bool contagion, double ratio){
        cout<<"Constructor reached"<<endl;
        int n=rewire*100.0;
        startingRatio=ratio;
        std::string rat=getSubFolderName(ratio);
        std::string cont;

        if(contagion)
            cont="ComplexContagion/";
        else
            cont="SimpleContagion/";
        if(infname.substr(0, 5)=="RealW"){
            std::string fname=infname.substr(10);
            this->inputFileName="../data/input/"+infname+".txt";
            this->outputFileName="../data/output/"+infname+"/"+cont+""+rat+fname+"_r"+std::to_string(n)+"_"+std::to_string(getRandomNumber(10000))+".txt";
        }
        else if(infname.substr(0, 5)=="Watts"){
            std::string pVal=infname.substr(infname.size()-2, 2)+"/";
            this->inputFileName="../data/input/WattsStrogatz/"+infname+".txt";
            this->outputFileName="../data/output/WattsStrogatz/"+cont+rat+pVal+"ws_r"+std::to_string(n)+"_"+std::to_string(getRandomNumber(10000))+".txt";
        }
        else if(infname.substr(0, 5)=="Erdos"){

        }
        stat0=0;
        stat1=0;
        epochLimit=10000000;
        stepCount=200;
        rewiringProbability=rewire; 
        relativeSize=0.5;
        complexContagion=contagion;
    }

    void loadData(){
        cout<<"Data Load Start"<<endl;
        fstream file;
        file.open(this->inputFileName, ios::in);
        string tp;
        int cc=0;
        while(getline(file, tp)){
            std::istringstream is(tp);
            int inputNode, outputNode;
            is>>inputNode;
            is>>outputNode;
            if(cc==0){
                cout<<"Start Node Generation"<<endl;
                generateNetwork(inputNode, outputNode);
                cc++;
            }
            else{
                if(inputNode==outputNode)
                    continue; //prevents edges into self
                Node* node1=getNode(inputNode);
                Node* node2=getNode(outputNode);
                node1->addNeighbour(node2);
                node2->addNeighbour(node1);
            }        
        }
        cout<<"Data Loaded"<<endl;
        file.close();
        cout<<"Nodes = "<<this->nodeCount<<"\tEdges = "<<this->edgeCount<<endl;
    }

    void generateNetwork(int nodeC, int edgeC){
        this->nodeCount=nodeC;
        this->edgeCount=edgeC;
        for(int i=0; i<nodeC; i++){
            double randState=getRandomNumber();
            Node* newNode;
            if(randState<this->startingRatio==0){
                stat0++;
                newNode=new Node(i, false);
            }
            else{
                stat1++;
                newNode=new Node(i, true);
            }
            
            this->nodeList.push_back(newNode);
        }
        cout<<"Nodes Generated"<<endl;
    }

    void convince(Node* inputNode, Node* outputNode){
        outputNode->setState(inputNode);
        if(outputNode->getState()){
            this->stat1++;
            this->stat0--;
        }
        else{
            this->stat0++;
            this->stat1--;
        }
    }

    Node* getNode(int identity){
        if(identity<0 || identity>=this->nodeCount)
            cout<<"Node with id = "<<identity<<" not found"<<endl;
        else
            return nodeList[identity];
        return nullptr;
    }


    int getRandomNumber(int limit){
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<int> dist(0, limit); 
        return dist(rng);
    }

    double getRandomNumber(){
        std::random_device rd;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rd);
    }

    Node* getRandomNode(){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, this->nodeCount-1);
        Node* node=this->nodeList[dist(gen)];
        if(node==nullptr){
            cout<<"getRandomNode() Error "<<dist(gen)<<endl;
            return nullptr;
        }
        return node;
    }

    std::string getSubFolderName(double ratio){
        switch((int)(ratio*10)) {
            case 1:
                return "01/";
            case 2:
                return "02/";
            case 3:
                return "03/";
            case 4:
                return "04/";
            case 5:
                return "05/";
        }

        return "NAHI";
    }

    void printAllNodes(){
        for(Node* node:nodeList){
            node->printAllNeighbours();
        }
    }
};

int main(){
    DynamicNetwork* network=new DynamicNetwork("RealWorld/trial data", 0.5, false, 0.5);
    network->loadData();
    network->printAllNodes();
    Node* n1=network->getNode(0);
    Node* n2=network->getNode(1);
    Node* n3=network->getNode(2);
    network->convince(n1, n2);
    network->convince(n1, n3);
    cout<<endl;
    network->printAllNodes();
    return 0;
}