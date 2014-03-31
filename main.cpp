#include <fstream>
#include <string>
#include <iostream>
#define HASH    997
#define AP_THRESHOLD_P1 200
#define AP_THRESHOLD_P2 90
#define IP_THRESHOLD    0.8
#define SCORE_THRESHOLD 1.05
using namespace std;

ifstream fin("data.csv");
ofstream fout("result.txt");

class action_log{
public:
	int user_id;
	int month, day;
	int action_type;
	int brand_id;
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
    struct node{
        int brand_id;
        int count[4];
        double score;
        double scale;
        bool decide;
        node *next;
        node(){
            next = NULL;
            count[0] = count[1] = count[2] = count[3] = 0;
            score = 0;
            scale = 0;
            decide = 0;
        }
    };

    node *list;

    user_struct(){
        list = new node;
        brand_sum = 0;
    }

    void insert_action(int b,int t){
        node *p;
        p = list;
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
        if (t!=-1) p->count[t]++;
        if (t==-1) {
            p->decide = 1;
        }
        brand_sum++;
        return;
    }

    void insert_scale(int b,double s){
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

    void make_decide(){
        int i,j,k;
        node *p;
        p = list;
        while (p->next!=NULL){
            p = p->next;
            k = p->count[2]+p->count[3];
            p->score = p->count[0]*p->scale;
            if ((p->score>=SCORE_THRESHOLD)|| (p->count[1]>1)  || ((k>0)&&(p->count[1] == 0)&&(p->score>SCORE_THRESHOLD/2))  ) {
                p->decide = 1;
            }
            else
                p->decide = 0;
        }
    }

    int output_user(){
        node *p;
        p = list;
        bool tmp = 1;
        int k = 0;
        int i = 0;
        while (p->next!=NULL){
            p = p->next;
            if (p->brand_id == 21110){
                i = 1;
            }
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

int log_num = 0, brand_num = 0, user_num = 0;
hash_struct brand_hash[HASH];
hash_struct user_hash[HASH];
brand_struct brand[10000];
user_struct user[10000];
action_log *log = new action_log[200000];

void define_new_id(){
    int i,j,k;
    i = 0;
    while (i<log_num){

        k = log[i].brand_id;
        k = brand_hash[k%HASH].check(k,brand_num);
        if (k == -1)brand_num++;
        k = log[i].brand_id;
        k = brand_hash[k%HASH].check(k,brand_num);
        brand[k].count[log[i].action_type]++;
        brand[k].brand_id = log[i].brand_id;

        k = log[i].user_id;
        k = user_hash[k%HASH].check(k,user_num);
        if (k == -1) user_num++;
        k = log[i].user_id;
        k = user_hash[k%HASH].check(k,user_num);
        user[k].user_id = log[i].user_id;
        user[k].insert_action(log[i].brand_id,log[i].action_type);
        i++;
    }

    double s;
    for (i=0;i<brand_num;i++){
        s = 1;
        if (brand[i].count[0]!=0)
            s = double(brand[i].count[1]) / double(brand[i].count[0]);
        for (j=0;j<user_num;j++){
            user[j].insert_scale(brand[i].brand_id,s);
        }
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

int input_log(){
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
		log[num].user_id = j;
		j = 0;
		for (i = i + 1; i < s.length(); i++){
			if (s[i] != ','){
				j = j * 10 + (s[i] - '0');
			}
			else break;
		}
		log[num].brand_id = j;
		j = 0;
		log[num].action_type = s[i + 1] - '0';
		log[num].month = s[i + 3] - '0';
		log[num].day = s[i + 6] - '0';
		if (s[i + 7] >= '0' && s[i + 7] <= '9'){
			log[num].day *= 10;
			log[num].day += s[i + 7] - '0';
		}
		num++;
	}

	return num;
}

int main(){
    log_num = input_log();
    define_new_id();
    first_decide();
    frequent();
    //output_frequent_count();
    //output_brand_count();
    output_user_count();
	return 0;
}
