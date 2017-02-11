// COMPILED ON MAC: g++ lab1.cpp -o lab1 

#include <unistd.h>	 // close
#include <math.h>       // pow
#include <iostream>     // cout
#include <string>       // stod
#include <vector>       // (size can change)
#include <algorithm>    // sort
#include <iomanip>      // setprecision
#include <sys/socket.h> // socket
#include <arpa/inet.h>  // inet_addr
#include <pthread.h>    // pthread_t
#include <sched.h>      // sched_yield
using namespace std;

/* ******************************   CLASS   ***************************** */
string parsing(string & s) {
    size_t pos = 0;
    string delimiter = ";";
    pos = s.find(delimiter);
    string token = s.substr(0, pos);
    s.erase(0, pos + delimiter.length());
    return token;
}

class Wine {
private:
    double fixed_acidity, volatile_acidity, citric_acid, residual_sugar, chlorides;
    int free_sulphur_dioxide, total_sulphur_dioxide;
    double density, pH, sulphates, alcohol, quality;
    string type;
public:

    Wine(string str) {
        fixed_acidity = stod(parsing(str));
        volatile_acidity = stod(parsing(str));
        citric_acid = stod(parsing(str));
        residual_sugar = stod(parsing(str));
        chlorides = stod(parsing(str));
        free_sulphur_dioxide = stoi(parsing(str));
        total_sulphur_dioxide = stoi(parsing(str));
        density = stod(parsing(str));
        pH = stod(parsing(str));
        sulphates = stod(parsing(str));
        alcohol = stod(parsing(str));
        quality = stod(parsing(str));
        type = str[0];
    }

    double getFixed_acidity() {
        return fixed_acidity;
    }

    void setFixed_acidity(double f) {
        fixed_acidity = f;
    }

    double getVolatile_acidity() {
        return volatile_acidity;
    }

    void setVolatile_acidity(double v) {
        volatile_acidity = v;
    }

    double getCitric_acid() {
        return citric_acid;
    }

    void setCitric_acid(double c) {
        citric_acid = c;
    }

    double getResidual_sugar() {
        return residual_sugar;
    }

    void setResidual_sugar(double r) {
        residual_sugar = r;
    }

    double getChlorides() {
        return chlorides;
    }

    void setChlorides(double ch) {
        chlorides = ch;
    }

    int getFree_sulphur_dioxide() {
        return free_sulphur_dioxide;
    }

    void setFree_sulphur_dioxide(int fr) {
        free_sulphur_dioxide = fr;
    }

    int getTotal_sulphur_dioxide() {
        return total_sulphur_dioxide;
    }

    void setTotal_sulphur_dioxide(int t) {
        total_sulphur_dioxide = t;
    }

    double getDensity() {
        return density;
    }

    void setDensity(double d) {
        density = d;
    }

    double getpH() {
        return pH;
    }

    void setpH(double p) {
        pH = p;
    }

    double getSulphates() {
        return sulphates;
    }

    void setSulphates(double s) {
        sulphates = s;
    }

    double getAlcohol() {
        return alcohol;
    }

    void setAlcohol(double a) {
        alcohol = a;
    }

    double getQuality() {
        return quality;
    }

    void setQuality(double q) {
        quality = q;
    }

    string getType() {
        return type;
    }

    void setType(string t) {
        type = t;
    }
};

/* ********************   DECLARATIONS   ********************* */
void menu();
void whiteStandardization(Wine w);
void redStandardization(Wine w);
double euclideanDistance(Wine w1, Wine w2);
string parsing(string & s);
void * dataset(void * args);
string typeClassification(Wine newWine);
void * predictType(void * args);
double qualityClassification(Wine newWine, int parameter);
void * predictQuality(void * args);
void parameterID(int i);

/* ***********************   GLOBAL   ************************ */
vector<Wine> wineList;
int choice, trainingset, k;
double marginoferror;
bool finished = false;

pthread_mutex_t data_mutex;
pthread_cond_t data_condition;
pthread_cond_t training_condition;

/* ***********************   MAIN   ************************ */
int main(int argc, char *argv[]) {

    if (argc < 2) {
        cerr << "Usage: <port_number>" << endl;
        exit(1);
    }

    menu();

    // ======================== Server Connection ========================     
    //Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0); // int socket(int domain, int type, int protocol);
    if (sock == -1) { // On success, a file descriptor for the new socket is returned. On error, -1 is returned, and errno is set appropriately
        cout << "\nCould not create socket!" << endl;
    }
    cout << "\nSocket created" << endl;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // 127.0.0.1 -> localhost (loopback)
    server.sin_port = htons(atoi(argv[1])); // porto utilizado -> 1234

    //Connect to remote server
    if (connect(sock, (struct sockaddr *) &server, sizeof (server)) < 0) {
        perror("Connect failed!");
        exit(1);
    }
    cout << "Connected" << endl;

    // ========================= PTHREADS =========================
    pthread_mutex_init(&data_mutex, NULL);
    pthread_cond_init(&data_condition, NULL);
    pthread_cond_init(&training_condition, NULL);

    pthread_t thread;
    pthread_create(&thread, NULL, dataset, (void *) &sock);

    // ========================== OUTPUT ==========================

    if (choice == 1) {
        int * success_count;
        pthread_t thread1;
        pthread_create(&thread1, NULL, predictType, NULL);
        pthread_join(thread1, (void **) &success_count);
        double success = ((double) *success_count / (wineList.size() - trainingset))*100;
        cout << "\n\t---OUTPUT---\nSuccess rate: " << setprecision(3) << success << "%\n" << endl;
    }

    if (choice == 2) {
        int thread_parameter[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        int * success_count[11];
        pthread_t threads[11];
        for (int i = 0; i <= 10; i++)
            pthread_create(&threads[i], NULL, predictQuality, (void *) &thread_parameter[i]);
        for (int i = 0; i <= 10; i++)
            pthread_join(threads[i], (void **) &success_count[i]);
        cout << "\n\t---OUTPUT---\n";
        for (int i = 0; i <= 10; i++) {
            parameterID(i);
            double success = ((double) *success_count[i] / (wineList.size() - trainingset))*100;
            cout << "Success rate: " << setprecision(3) << success << "%" << endl;
        }
    }

    cout << "\nK=" << k << "  |  Training Set: " << trainingset << "  |  Test Set: " << wineList.size() - trainingset << "  |  TOTAL: " << wineList.size();
    if (choice == 2)
        cout << "  |  Margin of error: " << marginoferror << "\n" << endl;

    // Clean up and exit 
    pthread_mutex_destroy(&data_mutex);
    pthread_cond_destroy(&data_condition);
    pthread_cond_destroy(&training_condition);

    return 0;
}
// TOTAL SEND BY SERVER: 1599 RED + 1479 WHITE = 3078

/* ***********************************************************************
 *******************************   GLOBAL   *******************************
 *********************************************************************** */

void menu() {
    cout << "\n ********   KNN Classification   ********" << endl;
    do {
        cout << "\n ---------   MENU   ---------\n1. Predict Wine Type\n2. Predict Wine Quality\n3. Exit" << endl;
        cout << "\nWhat do you pretend? ";
        cin >> choice;
        if (choice < 1 || choice > 3 || !cin) {
            cin.clear();
            cout << "\nChoose one of the options!" << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if (choice == 3) {
            cout << "Until next time!" << endl;
            exit(0);
        }
    } while (choice < 1 || choice > 3 || !cin);

    do {
        cout << "\nSize of the training set: ";
        cin >> trainingset;
        if (trainingset < 100 || trainingset > 2500 || !cin) {
            cout << "The training set must be between 100 and 2500!" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (trainingset < 100 || trainingset > 2500 || !cin);

    do {
        cout << "\nK-value: ";
        cin >> k;
        if (k < 1 || k % 2 == 0 || !cin) {
            cout << "K must be an odd number greater than or equal to 1!" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    } while (k < 1 || k % 2 == 0 || !cin);

    if (choice == 2)
        do {
            cout << "\nMargin of error: ";
            cin >> marginoferror;
            if (marginoferror < 0 || marginoferror > 1 || !cin) {
                cout << "The margin of error must be between 0 and 1!" << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } while (marginoferror < 0 || marginoferror > 1 || !cin);
}

void * dataset(void * args) {
    int * sock = (int *) args;
    char server_reply[512];
    while (1) {
        pthread_mutex_lock(&data_mutex);
        int numBytesRead = recv(*sock, server_reply, sizeof (server_reply), 0);
        strtok(server_reply, " \n");
        cout << "\tSERVER: " << server_reply << endl;
        string s(server_reply);
        if (numBytesRead < 0) {
            cout << "\nrecv failed!" << endl;
            exit(1);
        }
        if (numBytesRead == 0) {
            cout << "\nData Set concluded!" << endl;
            break;
        }
        wineList.push_back(Wine(s));
        if (choice == 1) { // on predicting type, the wines are standardize
            if (wineList.back().getType() == "W")
                whiteStandardization(wineList.back());
            else
                redStandardization(wineList.back());
        }
        if (wineList.size() == trainingset) {
            pthread_cond_broadcast(&training_condition);
            cout << "\nTraining Set concluded!" << endl;
        }
        pthread_cond_signal(&data_condition);
        pthread_mutex_unlock(&data_mutex);
    }
    finished = true;
    pthread_cond_broadcast(&data_condition);
    pthread_mutex_unlock(&data_mutex);
    close(*sock);
    pthread_exit(NULL);
}

/* *********************************************************************
 *************************   TYPE PREDICTION   **************************
 ********************************************************************* */

void * predictType(void * args) {
    string type;
    int i = 0;
    static int success_count = 0;
    do {
        sched_yield();
        pthread_mutex_lock(&data_mutex);
        while (wineList.size() <= trainingset)
            pthread_cond_wait(&training_condition, &data_mutex);
        if (finished == false)
            pthread_cond_wait(&data_condition, &data_mutex);
        type = typeClassification(wineList[trainingset + i]);
        cout << "REAL: " << wineList[trainingset + i].getType() << "\tPREDICTION: " << type << endl;
        if (type == wineList[trainingset + i].getType())
            success_count++;
        i++;
        pthread_mutex_unlock(&data_mutex);
    } while (i < wineList.size() - trainingset);
    return (void*) &success_count;
}

string typeClassification(Wine newWine) {
    vector<pair<double, int> > distanceList;
    // makes a pair of distance / index in wineList
    for (int i = 0; i < trainingset; i++) {
        double dist = euclideanDistance(wineList[i], newWine);
        distanceList.push_back(make_pair(dist, i));
    }
    sort(distanceList.begin(), distanceList.end());

    vector<pair<double, int> >::iterator it;
    int white = 0;
    int red = 0;
    int counterK = 0;
    // looks for knn and count how many there are of each type 
    for (it = distanceList.begin(); it != distanceList.end(); it++) {
        string type = wineList[it->second].getType();
        //cout << "\t\t INDICE:  " << it->second << "\t TYPE:  " << type <<  endl;
        if (type == "W")
            white++;
        else if (type == "R")
            red++;
        counterK++;
        if (counterK == k)
            break;
    }
    return white > red ? "W" : "R";
}

void whiteStandardization(Wine w) {
    w.setFixed_acidity((w.getFixed_acidity() - 6.854788) / 0.843868);
    w.setVolatile_acidity((w.getVolatile_acidity() - 0.278241) / 0.100795);
    w.setCitric_acid((w.getCitric_acid() - 0.334192) / 0.121020);
    w.setResidual_sugar((w.getResidual_sugar() - 6.391415) / 5.072058);
    w.setChlorides((w.getChlorides() - 0.045772) / 0.021848);
    w.setFree_sulphur_dioxide((w.getFree_sulphur_dioxide() - 35.308085) / 17.007137);
    w.setTotal_sulphur_dioxide((w.getTotal_sulphur_dioxide() - 138.360657) / 42.498065);
    w.setDensity((w.getDensity() - 0.994027) / 0.002991);
    w.setpH((w.getpH() - 3.188267) / 0.151001);
    w.setSulphates((w.getSulphates() - 0.489847) / 0.114126);
    w.setAlcohol((w.getAlcohol() - 10.514267) / 1.230621);
}

void redStandardization(Wine w) {
    w.setFixed_acidity((w.getFixed_acidity() - 8.319637) / 1.741096);
    w.setVolatile_acidity((w.getVolatile_acidity() - 0.527821) / 0.179060);
    w.setCitric_acid((w.getCitric_acid() - 0.270976) / 0.194801);
    w.setResidual_sugar((w.getResidual_sugar() - 2.538806) / 1.409928);
    w.setChlorides((w.getChlorides() - 0.087467) / 0.047065);
    w.setFree_sulphur_dioxide((w.getFree_sulphur_dioxide() - 15.874922) / 10.460157);
    w.setTotal_sulphur_dioxide((w.getTotal_sulphur_dioxide() - 46.467792) / 32.895324);
    w.setDensity((w.getDensity() - 0.996747) / 0.001887);
    w.setpH((w.getpH() - 3.311113) / 0.154386);
    w.setSulphates((w.getSulphates() - 0.658149) / 0.169507);
    w.setAlcohol((w.getAlcohol() - 10.422983) / 1.065668);
}

double euclideanDistance(Wine w1, Wine w2) {
    double sum = pow(w1.getFixed_acidity() - w2.getFixed_acidity(), 2) +
            pow(w1.getVolatile_acidity() - w2.getVolatile_acidity(), 2) +
            pow(w1.getCitric_acid() - w2.getCitric_acid(), 2) +
            pow(w1.getResidual_sugar() - w2.getResidual_sugar(), 2) +
            pow(w1.getChlorides() - w2.getChlorides(), 2) +
            pow(w1.getFree_sulphur_dioxide() - w2.getFree_sulphur_dioxide(), 2) +
            pow(w1.getTotal_sulphur_dioxide() - w2.getTotal_sulphur_dioxide(), 2) +
            pow(w1.getDensity() - w2.getDensity(), 2) +
            pow(w1.getpH() - w2.getpH(), 2) +
            pow(w1.getSulphates() - w2.getSulphates(), 2) +
            pow(w1.getAlcohol() - w2.getAlcohol(), 2);

    return sqrt(sum);
}

/* *********************************************************************
 ************************   QUALITY PREDICTION   ************************
 ********************************************************************* */

void * predictQuality(void * args) {
    int * parameter = (int *) args;
    double quality;
    int i = 0;
    int success_count = 0;
    do {
        if (finished == false)
            sched_yield();
        pthread_mutex_lock(&data_mutex);
        while (wineList.size() <= trainingset)
            pthread_cond_wait(&training_condition, &data_mutex);
        if (finished == false)
            pthread_cond_wait(&data_condition, &data_mutex);
        quality = qualityClassification(wineList[trainingset + i], *parameter);
        if (quality >= (wineList[trainingset + i].getQuality() - marginoferror) && quality <= (wineList[trainingset + i].getQuality() + marginoferror))
            success_count++;
        parameterID(*parameter);
        cout << "REAL: " << wineList[trainingset + i].getQuality() << "\tPREDICTION: " << quality << "\tERROR: " << wineList[trainingset + i].getQuality() - quality << endl;
        i++;
        pthread_mutex_unlock(&data_mutex);
    } while (i < wineList.size() - trainingset);
    int * count = (int *) malloc(sizeof (int));
    * count = success_count;
    return (void*) count;
}

double qualityClassification(Wine newWine, int parameter) {
    double wine1;
    double wine2;
    vector<pair<double, int> > distanceList;
    for (int i = 0; i < trainingset; i++) {
        switch (parameter) {
            case 0:
                wine1 = wineList[i].getFixed_acidity();
                wine2 = newWine.getFixed_acidity();
                break;
            case 1:
                wine1 = wineList[i].getVolatile_acidity();
                wine2 = newWine.getVolatile_acidity();
                break;
            case 2:
                wine1 = wineList[i].getCitric_acid();
                wine2 = newWine.getCitric_acid();
                break;
            case 3:
                wine1 = wineList[i].getResidual_sugar();
                wine2 = newWine.getResidual_sugar();
                break;
            case 4:
                wine1 = wineList[i].getChlorides();
                wine2 = newWine.getChlorides();
                break;
            case 5:
                wine1 = wineList[i].getFree_sulphur_dioxide();
                wine2 = newWine.getFree_sulphur_dioxide();
                break;
            case 6:
                wine1 = wineList[i].getTotal_sulphur_dioxide();
                wine2 = newWine.getTotal_sulphur_dioxide();
                break;
            case 7:
                wine1 = wineList[i].getDensity();
                wine2 = newWine.getDensity();
                break;
            case 8:
                wine1 = wineList[i].getpH();
                wine2 = newWine.getpH();
                break;
            case 9:
                wine1 = wineList[i].getSulphates();
                wine2 = newWine.getSulphates();
                break;
            case 10:
                wine1 = wineList[i].getAlcohol();
                wine2 = newWine.getAlcohol();
                break;
        }
        double dist = sqrt(pow(wine1 - wine2, 2));
        distanceList.push_back(make_pair(dist, i));
    }
    sort(distanceList.begin(), distanceList.end());

    vector<pair<double, int> >::iterator it;
    vector<int> qualityList;
    int counterK = 0;
    for (it = distanceList.begin(); it != distanceList.end(); it++) { // finds knn quality
        qualityList.push_back(wineList[it->second].getQuality());
        counterK++;
        if (counterK == k)
            break;
    }
    /*
    cout << "\n\tqualityList = { " ;
    for(int i=0; i<qualityList.size(); i++)
        cout <<  qualityList[i] << ", "; 
    cout << "}" << endl;
     */
    vector<pair<int, int> > freqList;
    for (int quality = 0; quality <= 10; quality++) { // creates a pair quality/frequency
        int freq = 0;
        for (int j = 0; j < qualityList.size(); j++) {
            if (qualityList[j] == quality)
                freq++;
        }
        if (freq != 0)
            freqList.push_back(make_pair(quality, freq));
    }
    /*
    cout << "\n\tfreqList(quality/frequency) = { " ;
    for(int i=0; i<freqList.size(); i++)
        cout << "(" << freqList[i].first << ", " << freqList[i].second << "), ";   
    cout << "}" << endl;
     */
    vector<pair<double, int> > meanList;
    for (int freq = k; freq >= 1; freq--) { // creates a pair (mean of quality with same frequency)/frequency
        double num = 0;
        int d = 0;
        for (int i = 0; i < freqList.size(); i++) {
            if (freq == freqList[i].second) {
                num += freqList[i].first;
                d++;
            }
        }
        if (d != 0)
            meanList.push_back(make_pair(num / d, freq));
    }
    /*
    cout << "\n\tmeanList(quality mean/frequency) = { " ;
    for(int i=0; i<meanList.size(); i++)
        cout << "(" << meanList[i].first << ", " << meanList[i].second << "), ";
    cout << "}" << endl;
     */
    return meanList[0].first;
}

void parameterID(int i) {
    switch (i) {
        case 0:
            cout << "\n- Fixed Acidity -> ";
            break;
        case 1:
            cout << "\n- Volatile Acidity -> ";
            break;
        case 2:
            cout << "\n- Citric Acid -> ";
            break;
        case 3:
            cout << "\n- Residual Sugar -> ";
            break;
        case 4:
            cout << "\n- Chlorides -> ";
            break;
        case 5:
            cout << "\n- Free Sulphur Dioxide -> ";
            break;
        case 6:
            cout << "\n- Total Sulphur Dioxide -> ";
            break;
        case 7:
            cout << "\n- Density -> ";
            break;
        case 8:
            cout << "\n- pH -> ";
            break;
        case 9:
            cout << "\n- Sulphates -> ";
            break;
        case 10:
            cout << "\n- Alcohol -> ";
            break;
    }
}