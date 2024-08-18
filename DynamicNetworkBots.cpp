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
    bool state, bot;
    std::vector<Node*> neighbours;

    public:

    Node(int identity, bool status, bool botus){
        this->id = identity;
        this->state = status;
        this->bot = botus;
    }

    //Sets the state of a node, equal to the state of another node passed as a parameter.
    bool setState(Node* node){
        if(!this->bot){
            this->state=node->getState();
            return true;
        }
        return false;
    }

    //Sets the state of a node to a bool value passed as a parameter
    bool setState(bool status){
        if(!this->bot){
            this->state=status;
            return true;
        }
        return false;
    }

    int getId(){
        return this->id;
    }

    //All bots will have state as '0'
    bool getState(){
        return this->state;
    }

    bool isBot(){
        return this->bot;
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

    //returns a random discordant neighbour
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

    //returns a random discordant neighbour which is not a bot
    Node* getDiscordantRealNeighbour(){
        std::vector<Node*> roster;
        for(Node* node: neighbours){
            if(node->getState()!=this->state && !node->isBot())
                roster.push_back(node);
        }
        if(roster.size()==0)
            return nullptr;

        return roster[this->getRandomNumber(roster.size()-1)];
    }

    int getDiscordantNeighbourCount(){
        int count=0;
        if(this->isBot())
            return 0;
        for(Node* node: neighbours){
            if(node->getState()!=this->state && !node->isBot())
                count++;
        }
        return count;
    }

    void addNeighbour(Node* newNeighbour){
        for(Node* node: neighbours){
            if(node->getId() == newNeighbour->getId()){
                //cout<<this->getId()<< " already has neighbour with ID=" <<newNeighbour->getId()<<". Ignoring duplicate addition."<<endl;
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

    bool isRealNeighbour(Node* node){
        if(node->isBot())
            return false;
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
    int nodeCount, edgeCount, stat0, stat1, botCount, botDegree, totalNodes;
    int epochLimit, stepCount;
    double rewiringProbability, relativeSize, startingRatio, botRatio;
    std::vector<Node*> nodeList;
    std::string inputFileName, outputFileName;
    long rew=0, con=0;
    bool complexContagion;

    DynamicNetwork(std::string infname, double rewire, double ratio, double botRatio, int botDegree){
        cout<<"Constructor reached"<<endl;
        int n=rewire*100.0;
        this->startingRatio=ratio;
        std::string rat=getSubFolderName(ratio);
        int brat=botRatio*100.0;

        if(infname.substr(0, 5)=="RealW"){
            std::string fname=infname.substr(10);
            this->inputFileName="../data/input/"+infname+".txt";
            this->outputFileName="../data/output/Bots/"+infname+"/"+rat+fname+"_r"+std::to_string(n)+"_br"+std::to_string(brat)+"_"+std::to_string(getRandomNumber(10000))+".txt";
        }
        else if(infname.substr(0, 5)=="Watts"){
            std::string pVal=infname.substr(infname.size()-2, 2)+"/";
            this->inputFileName="../data/input/WattsStrogatz/"+infname+".txt";
            this->outputFileName="../data/output/Bots/WattsStrogatz/"+rat+pVal+"ws_r"+std::to_string(n)+"_br"+std::to_string(brat)+"_"+std::to_string(getRandomNumber(10000))+".txt";
        }
        else if(infname.substr(0, 5)=="Erdos"){

        }
        stat0=0;
        stat1=0;
        epochLimit=10000000;
        stepCount=50;
        this->rewiringProbability=rewire; 
        this->relativeSize=0.5;
        this->botRatio=botRatio;
        this->botDegree=botDegree;
        cout<<outputFileName<<endl;
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
                this->generateNetwork(inputNode, outputNode);
                cc++;
            }
            else{
                if(inputNode==outputNode)
                    continue; //prevents edges into self
                double ran=this->getRandomNumber();
                if(ran<=0.9){ //this ensures that every network iteration is slightly different
                    Node* node1=getNode(inputNode);
                    Node* node2=getNode(outputNode);
                    node1->addNeighbour(node2);
                    node2->addNeighbour(node1);
                }
            }        
        }
        cout<<"Data Loaded"<<endl;
        this->connectBots();
        file.close();
        cout<<"Nodes = "<<this->nodeCount<<"\tEdges = "<<this->edgeCount<<endl;
    }

    void generateNetwork(int nodeC, int edgeC){
        this->nodeCount=nodeC;
        this->edgeCount=edgeC;
        this->botCount=nodeC*this->botRatio;
        int i=0;
        for(i=0; i<nodeC; i++){
            double randState=getRandomNumber();
            Node* newNode;
            if(randState<this->startingRatio){
                stat0++;
                newNode=new Node(i, false, false);
            }
            else{
                stat1++;
                newNode=new Node(i, true, false);
            }
            this->nodeList.push_back(newNode);
        }
        cout<<"Nodes Generated"<<endl;
        for(;this->botCount>0; this->botCount--, i++){
            Node* newNode= new Node(i, false, true);
            this->nodeList.push_back(newNode);
            stat0++;
        }
        this->totalNodes=i;
        this->botCount=nodeC*this->botRatio;
    }

    void connectBots(){
        for(Node* botNode: nodeList){
            if(!botNode->isBot())
                continue;
            
            for(int i=0;i<this->botDegree;i++){
                Node* newNode=this->getRandomNode();
                if(!botNode->isNeighbour(newNode) && !newNode->isBot()){
                    botNode->addNeighbour(newNode);
                    newNode->addNeighbour(botNode);
                }
            }
        }
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
            node=this->getAnyRandomNode();
            if(node->hasDiscordantEdge())
                ideal=true;
            tries++;
        }while(!ideal && tries<2000);
        
        if(tries>=2000){
            cout<<"Tries is greater with a value of "<<tries<<endl;
            return true;
        }

        if(rando<=rewiringProbability){
            Node* neighbour=node->getDiscordantNeighbour();
            Node* newNeighbour=this->getNewSimilarNeighbour(node);
            if(newNeighbour==nullptr || neighbour==nullptr)
                return false;
            this->rewire(node, neighbour, newNeighbour);
        }
        else{ 
            Node* neighbour=node->getDiscordantNeighbour();
            this->convince(node, neighbour);
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
            Node* newNeighbour=this->getNewSimilarNeighbour(node1);
            if(newNeighbour==nullptr)
                return false;
            this->rewire(node1, node2, newNeighbour);
        }
        else{ 
            Node* node2=node1->getDiscordantNeighbour();
            this->convince(node1, node2);
        }

        return false;
    }

    void convince(Node* inputNode, Node* outputNode){
        bool reply=outputNode->setState(inputNode);
        if(!reply)
            return;
        if(outputNode->getState()){
            this->stat1++;
            this->stat0--;
        }
        else{
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
        if(identity<0 || identity>=this->totalNodes)
            cout<<"Node with id = "<<identity<<" not found"<<endl;
        else
            return nodeList[identity];
        return nullptr;
    }

    //returns a new node which has same state as node passed in the parameter
    Node* getNewSimilarNeighbour(Node* node){
        int count=10000;
        Node* newNode;
        do{
            newNode=this->getAnyRandomNode();
            if(!node->isNeighbour(newNode) && newNode->getState()==node->getState() && node->getId()!=newNode->getId())
                return newNode;
            count--;
        }while(count>0);
        return nullptr;
    }

    int getDiscordantEdgeCount(){
        int count=0;
        for(Node* node: nodeList)
            count+=node->getDiscordantNeighbourCount();
        
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

    Node* getAnyRandomNode(){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, this->totalNodes-1);
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
    double rewiring[]={0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    double start[]={0.1, 0.2, 0.3, 0.4, 0.5, 0.1, 0.2, 0.3, 0.4, 0.5};
    bool contagi[]={false};
    std::string networks[]={"RealWorld/facebook"};
    //std::string network[]={"RealWorld/twitchDE", "RealWorld/twitchENGB", "RealWorld/twitchES", "RealWorld/twitchFR", "RealWorld/twitchPTBR", "RealWorld/twitchRU", "RealWorld/astroPh", "RealWorld/emailEU", "RealWorld/hepPh", "RealWorld/lastfm"};
    int l1=sizeof(rewiring)/sizeof(rewiring[0]);
    int l2=sizeof(start)/sizeof(start[0]);
    int l3=sizeof(contagi)/sizeof(contagi[0]);
    int l4=sizeof(networks)/sizeof(networks[0]);

    DynamicNetwork* nw=new DynamicNetwork("RealWorld/facebook", 0.4, 0.4, 0.05, 20);
    nw->loadData();
    nw->printAllNodes();
    cout<<nw->getSummary(10, 10)<<endl;
    cout<<nw->outputFileName<<endl;
    nw->beginSimulation();
    
    
    int execution=0;
    // for(std::string n: networks){
    //     for(bool c: contagi){
    //         for(double  r : rewiring){
    //             for(double st: start){
    //                 DynamicNetwork* network=new DynamicNetwork(n, r, st, 0.05, 20);
    //                 network->loadData();
    //                 network->beginSimulation();
    //                 execution++;
    //                 //network->printAllEdges();
    //                 //network->checkDegreeDistribution();
    //                 cout<<"Completed for r="<<r<<" Execution = "<<execution<<"/"<<l1*l2*l3*l4<<endl;
    //                 //network->printAllEdges(500);
    //                 delete network;
    //             }
    //         }
    //     }
    // }
    return 0;
}