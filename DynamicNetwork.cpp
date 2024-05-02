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

    void setState(bool status){
        this->state=status;
    }

    int getId(){
        return this->id;
    }

    bool getState(){
        return this->state;
    }

    const std::vector<Node*>& getNeighbours() const {
        return neighbours;
    }

    Node* getNeighbour(int id){
        for(Node* node: neighbours){
            if(node->getId()==id)
                return node;
        }
        cout<<"Neighbour ID="<<id<<" not found."<<endl;
        return nullptr;
    }

    Node* getDiscordantNeighbour(){
        std::vector<Node*> roster;
        for(Node* node: neighbours){
            if(node->getState()!=this->state)
                roster.push_back(node);
        }
        if(roster.size()==0)
            return nullptr;

        return roster[this->getRandomNumber(roster.size()-1)];
    }

    int getDiscordantEdgeCount(){
        int count=0;
        for(Node* node: neighbours){
            if(node->getState()!=this->state)
                count++;
        }
        return count;
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

    bool isNeighbour(Node* node){
        for(Node* n: neighbours){
            if(node->getId() == n->getId())
                return true;
        }
        return false;
    }

    void printAllNeighbours(){
        cout<<this->id<<" ("<<this->state<<")-> ";
        for(Node* node: neighbours){
            cout<<node->getId()<<" ";
        }
        cout<<endl;
    }

    bool hasDiscordantEdge(){
        for(Node* node: neighbours)
            if(node->getState()!=this->state)
                return true;
        return false;
    }

    int getRandomNumber(int limit){
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<int> dist(0, limit); 
        return dist(rng);
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

    void beginSimulation(){
        long discEdge=this->getDiscordantEdgeCount();
        bool altEdgeSelectionAlgo=false;
        cout<<getSummary(-1, discEdge)<<endl;
        ofstream outputFile;
        outputFile.open(this->outputFileName);
        outputFile<<"Epoch Pop Frac DiscEdge"<<endl;
        for(int epoch=0; epoch<this->epochLimit; epoch++){
            for(int step=0; step<this->stepCount; step++){
                bool areWeDone=false;
                if(!altEdgeSelectionAlgo)
                    altEdgeSelectionAlgo=interact();
                else
                    areWeDone=interactAlt();
                if(areWeDone){
                    discEdge=this->getDiscordantEdgeCount();
                    std::string summary=getSummary(epoch, discEdge);
                    cout<<summary<<endl;
                    outputFile << summary <<endl;
                    this->recountStates();
                    cout<<"Simulation Completed!"<<endl;
                    outputFile.close();
                    return;
                }
            }
            if(epoch%10==0){
                cout<<"Epoch No. "<<epoch<<endl;
            }
            discEdge=this->getDiscordantEdgeCount();
            if(discEdge==0){
                std::string summary=getSummary(epoch, discEdge);
                cout<<summary<<endl;
                outputFile << summary <<endl;
                this->recountStates();
                cout<<"Simulation Completed!"<<endl;
                outputFile.close();
                return;
            }
            std::string summary=getSummary(epoch, discEdge);
            cout<<summary<<endl;
            outputFile << summary <<endl;

            if(discEdge<100 && !altEdgeSelectionAlgo){
                altEdgeSelectionAlgo=true;
                cout<<"Switching Algorithms"<<endl;
            }
        }
        outputFile.close();
    }

    bool interact(){
        bool ideal=false;
        double rando=this->getRandomNumber();
        Node* node;
        int tries=0;
        do{
            node=this->getRandomNode();
            if(node->hasDiscordantEdge())
                ideal=true;
            tries++;
        }while(!ideal && tries<10000);
        
        if(tries>=1000){
            cout<<"Tries is greater with a value of "<<tries<<endl;
            return true;
        }

        if(rando<=rewiringProbability){
            Node* neighbour=node->getDiscordantNeighbour();
            Node* newNeighbour=this->getNewNeighbour(node);
            this->rewire(node, neighbour, newNeighbour);
        }
        else{ 
            if(!this->complexContagion){
                Node* neighbour=node->getDiscordantNeighbour();
                this->convince(node, neighbour);
            }
            else
                this->convinceComplexContagion(node);
        }
        return false;
    }

    bool interactAlt(){
        std::vector<Node*> roster;
        double rando=this->getRandomNumber();
        for(Node* node: nodeList){
            if(node->getDiscordantNeighbour())
                roster.push_back(node);
        }
        if(roster.size()<=1 || this->getDiscordantEdgeCount()<=1){
            this->recountStates();
            return true;
        }
        int rand=this->getRandomNumber(roster.size()-1);
        //cout<<"random "<<rand<<" "<<edges.size()<<endl;
        Node* node1=roster[rand];
        if(rando<=rewiringProbability){
            Node* node2=node1->getDiscordantNeighbour();
            Node* newNeighbour=this->getNewNeighbour(node1);
            this->rewire(node1, node2, newNeighbour);
        }
        else{ 
            if(!this->complexContagion){
                Node* node2=node1->getDiscordantNeighbour();
                this->convince(node1, node2);
            }
            else
                this->convinceComplexContagion(node1);
        }

        return false;
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

    void convinceComplexContagion(Node* node){
        int op0=0, op1=0;
        for(Node* neighbour: node->getNeighbours()){
            if(neighbour->getState())
                op1++;
            else
                op0++;
        }
        double switchToTrue=op1/(op1+op0);
        double rando=this->getRandomNumber();
        if(rando<=switchToTrue){
            node->setState(true);
            this->stat1++;
            this->stat0--;
        }
        else{
            node->setState(false);
            this->stat0++;
            this->stat1--;
        }
    }

    void rewire(Node* node, Node* oldNeighbour, Node* newNeighbour){
        node->deleteNeighbour(oldNeighbour);
        oldNeighbour->deleteNeighbour(node);

        node->addNeighbour(newNeighbour);
        newNeighbour->addNeighbour(node);
    }

    Node* getNode(int identity){
        if(identity<0 || identity>=this->nodeCount)
            cout<<"Node with id = "<<identity<<" not found"<<endl;
        else
            return nodeList[identity];
        return nullptr;
    }

    Node* getNewNeighbour(Node* node){
        bool ideal=false;
        int count=10000;
        Node* newNode;
        do{
            newNode=this->getRandomNode();
            if(!node->isNeighbour(newNode))
                return newNode;
        }while(!ideal);
    }

    int getDiscordantEdgeCount(){
        int count=0;
        for(Node* node: nodeList)
            count+=node->getDiscordantEdgeCount();
        
        return count;
    }


    void recountStates(){
        int s1=0, s2=0;
        for(Node* node: nodeList){
            if(node->getState()==0)
                s1++;
            else
                s2++;
        }
        if(s1!=this->stat0 || s2!=this->stat1)
            cout<<"Counting Mismatch"<<endl;
    }

    std::string getSummary(int epoch, long discEdge){
        std::ostringstream oss;
        oss<<(epoch+1)<<" "<<stat0<<" "<<(stat0/(stat1*1.0+stat0*1.0))<<" "<<discEdge;
        return oss.str();
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
    Node* n3=network->getNode(5);
    network->rewire(n1, n2, n3);
    cout<<endl;
    network->printAllNodes();
    return 0;
}