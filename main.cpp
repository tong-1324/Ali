#include <fstream>
#include <string>
#include <iostream>
#include <math.h>
#define HASH    997
#define AP_THRESHOLD_P1 200
#define AP_THRESHOLD_P2 90
#define IP_THRESHOLD    0.8
#define SCORE_THRESHOLD 1.05
#define DIS_FACTOR 0.042
#define STD_FACTOR  0.42
using namespace std;

ifstream fin("data.csv");
ofstream fout("result.txt");

class action_logg{
public:
	int user_id;
	int month, day;
	int action_type;
	int brand_id;
};

struct vector{
    double count[4];
};

class hash_struct{
    struct node{
        int orign_id,new_id;
        node *next;
        node(): next(NULL){}
    };
    node *head;

    public:
    hash_struct(){
        head = new node;
    }

    int check(int num,int total){
        node *p;
        p = head;
        while (p->next!=NULL){
            p = p->next;
            if (p->orign_id==num) return p->new_id;
        }
        p->next = new node;
        p = p->next;
        p->orign_id = num;
        p->new_id = total;
        return -1;
    }

};

class brand_struct{
    public:
    int count[4];
    int brand_id;

    brand_struct(){
        count[0] = count[1] = count[2] = count[3] = 0;
    }
};

class user_struct{
    public:
    int user_id;
    int brand_sum;
    int action_num;
    vector standard;
    struct node{
        int brand_id;
        int count[4];
        double score;
        vector scale;
        bool decide;
        node *next;
        node(){
            next = NULL;
            count[0] = count[1] = count[2] = count[3] = 0;
            score = 0;
            scale.count[0] = scale.count[1] =scale.count[2] =scale.count[3] =0;
            decide = 0;
        }
    };

    node *list;

    user_struct(){
        list = new node;
        brand_sum = 0;
        action_num = 0;
        standard.count[0] = standard.count[1] = standard.count[2] = standard.count[3];
    }

    void insert_action(int b,int t){
        node *p;
        p = list;
        action_num++;
        while (p->next!=NULL){
            p = p->next;
            if (p->brand_id==b){
                p->count[t]++;
                return;
            }
        }
        p->next = new node;
        p = p->next;
        p->brand_id = b;
        p->count[t]++;
        brand_sum++;
        return;
    }

    void insert_scale(int b,vector s){
        node *p;
        p = list;
        while (p->next!=NULL){
            p = p->next;
            if (p->brand_id==b){
                p->scale = s;
                return;
            }
        }
    }

    void insert_pair(int id1,int id2){
        node *p;
        p = list;
        int tmp1,tmp2;
        double id1_s,id2_s;
        tmp1 = tmp2 = 1;
        while (p->next!=NULL){
            p = p->next;
            if (p->brand_id == id1){
                tmp1 = 0;
                id1_s = p->score;
                if ((tmp2 == 0) && (id2_s>=IP_THRESHOLD)) p->decide = 1;
            }
            if (p->brand_id == id2){
                tmp2 = 0;
                id2_s = p->score;
                if ((tmp1 == 0) && (id1_s>=IP_THRESHOLD)) p->decide = 1;
            }
        }
        if ((tmp1+tmp2)!=1) return;
        int tmp;
        if (tmp1==1){
            if (id2_s<IP_THRESHOLD) return;
            tmp = id1;
        }
        else{
            if (id1_s<IP_THRESHOLD) return;
            tmp = id2;
        }
        p->next = new node;
        p->brand_id = tmp;
        p->decide = 1;
        brand_sum++;
    }

    //get info about the d-th brand related to the user
    //return the brand_id if t==-1
    //return count[t] if t>0
    int info(int d,int t){
        node *p;
        p = list;
        int i,j,k;
        k = 0;
        while(k<d){
            k++;
            p = p->next;
        }
        if (t==-1) return p->brand_id;
        return p->count[t];
    }

    vector brand_vector(int b){
        node *p;
        p = list;
        int i,j,k;
        vector s;
        s.count[0] = s.count[1] = s.count[2] = s.count[3] = 0;
        while (p->next!=NULL){
            p = p->next;
            if (p->brand_id == b){
                for (i=0;i<4;i++) s.count[i] = p->count[i];
                return s;
            }
        }
        return s;
    }

    void make_standard(){
        node *p;
        p = list;
        int i,j,k = 0;
        double tmp;
        while (p->next!=NULL){
            p = p->next;
            if (p->count[1]>0){
                for (i=0;i<4;i++) standard.count[i] = standard.count[i] + p->count[i];
                k = k + 1;
            }
        }
        tmp = standard.count[1];
        for (i=0;i<4;i++) standard.count[i] = (standard.count[i]) / tmp;
    }

    void make_decide(){
        int i,j,k;
        double dis,tmp;
        node *p;
        p = list;
        while (p->next!=NULL){
            p = p->next;
            if (p->count[1]>1){
                p->decide = 1;
                continue;
            }
            j = k = 0;
            for (i=0;i<4;i++){
                if (i==1) continue;
                if (standard.count[i]<=p->count[i]*STD_FACTOR) k++;
                if (p->scale.count[i]<=p->count[i]*STD_FACTOR) j++;
            }
            if ((k==3)||(j==3)){
                p->decide = 1;
                continue;
            }
            dis = tmp = 0;
            for (i=0;i<4;i++){
                if (i==1) continue;
                dis = dis + (p->count[i]- standard.count[i]) * (p->count[i]- standard.count[i]);
                tmp = tmp + standard.count[i]*standard.count[i];
            }
            dis = sqrt(dis); tmp = sqrt(tmp);
            if (dis<tmp * DIS_FACTOR){
                p->decide = 1;
                continue;
            }
            dis = tmp = 0;
            for (i=0;i<4;i++){
                if (i==1) continue;
                dis = dis + (p->count[i]- p->scale.count[i]) * (p->count[i]- p->scale.count[i]);
                tmp = tmp + p->scale.count[i]*p->scale.count[i];
            }
            dis = sqrt(dis); tmp = sqrt(tmp);
            if (dis<tmp * DIS_FACTOR){
                p->decide = 1;
                continue;
            }
            p->decide = 0;
        }
    }

    int output_user(){
        node *p;
        p = list;
        bool tmp = 1;
        int k = 0;
        while (p->next!=NULL){
            p = p->next;
            if (p->decide == 1){
                if (tmp == 1){
                    tmp = 0;
                    fout<<user_id<<"\t";
                }
                else fout<<",";
                fout<<p->brand_id;
                k = k + 1;
            }
        }
        if (tmp == 1) return 0;
        fout<<"\n";
        return k;
    }

};

class frequent_item{
    public:
    int sum,brand_id;

        frequent_item(){
            sum = 0;
        }

        void insert(int b,int s){
            sum = s;
            brand_id = b;
        }
};

class frequent_pair{
    public:
    int id1,id2;
    int sum;

        frequent_pair(){
            sum = 0;
        }

        void insert(int a,int b,int s){
            sum = s;
            id1 = a;
            id2 = b;
        }

        void output(){
            fout<<id1<<","<<id2<<endl;
        }
};

int items_num = 0, pairs_num = 0;
frequent_item items[3000];
frequent_pair pairs[10000];

int logg_num = 0, brand_num = 0, user_num = 0;
hash_struct brand_hash[HASH];
hash_struct user_hash[HASH];
brand_struct brand[10000];
user_struct user[10000];
action_logg *logg = new action_logg[200000];

void define_new_id(){
    int i,j,k;
    i = 0;
    while (i<logg_num){

        k = logg[i].brand_id;
        k = brand_hash[k%HASH].check(k,brand_num);
        if (k == -1)brand_num++;
        k = logg[i].brand_id;
        k = brand_hash[k%HASH].check(k,brand_num);
        brand[k].count[logg[i].action_type]++;
        brand[k].brand_id = logg[i].brand_id;

        k = logg[i].user_id;
        k = user_hash[k%HASH].check(k,user_num);
        if (k == -1) user_num++;
        k = logg[i].user_id;
        k = user_hash[k%HASH].check(k,user_num);
        user[k].user_id = logg[i].user_id;
        user[k].insert_action(logg[i].brand_id,logg[i].action_type);
        i++;
    }
}

void scale(){
    vector s,t;
    int i,j,k;
    double tmp;
    for (i=0;i<brand_num;i++){
        k = 0;
        for (j=0;j<user_num;j++){
            t = user[j].brand_vector(brand[i].brand_id);
            if (t.count[1]>0){
                k++;
                s.count[0] = s.count[0] + t.count[0];
                s.count[1] = s.count[1] + t.count[1];
                s.count[2] = s.count[2] + t.count[2];
                s.count[3] = s.count[3] + t.count[3];
            }
        }
        tmp = s.count[1];
        for (j=0;j<4;j++) s.count[j] = (double(s.count[j])) / tmp;
        for (j=0;j<user_num;j++){
            user[j].insert_scale(brand[i].brand_id,s);
        }
    }

    for (i=0;i<user_num;i++){
        user[i].make_standard();
    }
}

void frequent(){
    int i,j,k,p,q,t,b,s;
    for (i=0;i<brand_num;i++){
        k = 0;
        for (j=0;j<4;j++){
            k = k + brand[i].count[j];
        }
        if (k>AP_THRESHOLD_P1){
            items[items_num].insert(brand[i].brand_id,k);
            items_num++;
        }
    }

    for (i=0;i<items_num;i++){
        for (j=i+1;j<items_num;j++){
            s = 0;
            for (k=0;k<user_num;k++){
                t = 0;
                for (p=0;p<user[k].brand_sum;p++){
                    b = user[k].info(p,-1);
                    if (b == items[i].brand_id) t++;
                    if (b == items[j].brand_id) t++;
                }
                if (t == 2){
                    s++;
                }
            }
            if (s>AP_THRESHOLD_P2){
                pairs[pairs_num].insert(items[i].brand_id,items[j].brand_id,s);
                pairs_num++;

                cout<<"now associting pair ("<<i<<","<<j<<")"<<endl;
                cout<<endl<<"total num : "<<s<<endl<<endl;
            }
        }
    }

    cout<<"total frequent pair: "<<pairs_num<<endl;

    for (i=0; i<pairs_num; i++){
        for (j=0; j<user_num; j++){
            user[j].insert_pair(pairs[i].id1, pairs[i].id2);
        }
    }

}
void output_user_logg(){
    int i,j,k;
    double s = 0;
    fout<<user_num<<endl;
    for (i=0;i<user_num;i++){
        //fout<<i<<" "<<user[i].action_num<<endl;
        s = s*i+user[i].action_num;
        s = s/(i+1);
    };
    k = 0;
    for (i=0;i<user_num;i++){
        if (user[i].action_num>s){
            fout<<i<<" "<<user[i].action_num<<endl;
            k++;
        }
    }
    fout<<k<<endl;
    fout<<s;
}
void output_brand_count(){
    int i,j,k;
    int sum = 0;
    i = 0;
    fout<<brand_num<<endl;
    while (i<brand_num){
        k = 0;
        for (j=0;j<4;j++){
            k = k + brand[i].count[j];
        }
        if (k<200) {
            i++;
            continue;
        }
        sum++;
        fout<<i<<"\t"<<brand[i].brand_id<<"\t"<<brand[i].count[0]<<"\t"<<brand[i].count[1]<<"\t"<<brand[i].count[2]<<"\t"<<brand[i].count[3]<<"\n";
        i++;
    }
    fout<<sum<<endl;
}

void output_frequent_count(){
    int i,j,k;
    fout<<pairs_num<<endl;
    for (i=0;i<pairs_num;i++) pairs[i].output();
}

void output_user_count(){
    int i,j,k;

    i = 0;
    k = 0;
    while (i<user_num){
        k = k + user[i].output_user();
        i++;
    }
    cout<<k<<endl;
    cin>>k;
}

void first_decide(){
    int i,j,k;

    for (i=0;i<user_num;i++){
        user[i].make_decide();
    }
}

int input_logg(){
    string s;
	fin >> s;
	int num = 0;

	while (!fin.eof()){
		fin >> s;
		int j = 0, i;
		for (i = 0; i < s.length(); i++){
			if (s[i] != ','){
				j = j * 10 + (s[i] - '0');
			}
			else break;
		}
		logg[num].user_id = j;
		j = 0;
		for (i = i + 1; i < s.length(); i++){
			if (s[i] != ','){
				j = j * 10 + (s[i] - '0');
			}
			else break;
		}
		logg[num].brand_id = j;
		j = 0;
		logg[num].action_type = s[i + 1] - '0';
		logg[num].month = s[i + 3] - '0';
		logg[num].day = s[i + 6] - '0';
		if (s[i + 7] >= '0' && s[i + 7] <= '9'){
			logg[num].day *= 10;
			logg[num].day += s[i + 7] - '0';
		}
		num++;
	}

	return num;
}

int main(){
    logg_num = input_logg();
    define_new_id();
    scale();
    first_decide();
    //frequent();
    //output_frequent_count();
    //output_brand_count();
    output_user_count();
    //output_user_logg();
	return 0;
}
