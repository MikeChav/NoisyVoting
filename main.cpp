#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

typedef vector<int> order;
#define _order const order &
#define vorder vector<order> // [i][j] indicates voter (i+1)'s (j+1)'s favorite candidate
#define UL unsigned long

uint64_t fact_c_1 = 0; // todo replace with gmp int


void print_vorder(const vorder &V) {
    cout << "QUICK" << endl;
    for (const auto& x : V) {
        if (x.empty()) cout << "--empty--";
        for (auto y : x) {
            cout << y << " ";
        }
        cout << endl;
    }
    cout << endl;
    cout << "CHECK" << endl;
}

struct problem_data {
    UL p, n /* num_voters*/, c/* num_candidates*/;
    vorder defaults;
    vector<double> sigmas;
    vector<double> randoms;
    vector<double> partitions;

    problem_data(UL n, UL c) : p(0), n(n), c(c) {
        sigmas = vector<double>(n);
        partitions = vector<double>(n);
        defaults = vorder(n);
        for (int i = 0; i < n; i++) {
            defaults[i] = order(c);
        }
    }
};

inline double dist(_order pi, _order pi_0) { //O(|pi|) = O(c)
    // in this implementation, we consider the following weight vector: <1, 0, ..., 0>
    // Thus the implementation reduces simply to:
    return abs(pi[0] - pi_0[0]);
}

void find_fact_c_1(UL c) { // O(c)
    if (fact_c_1 == 0) {
        fact_c_1 = 1;
        for (int i = 2; i < c; i++) {
            fact_c_1 *= i;
        }
    }
}

double calculate_partition(_order pi_0, double sigma) { // O(c)
    // this function is very easy to implement in O(c) time since the distance function only returns one of c value
    // note this is done per voter
    double Z = 0.0;
    for (int i = 1; i <= pi_0.size(); i++) {
        Z += exp(-1*sigma*abs(i-pi_0[0]));
    }
    return Z * fact_c_1;
}

void generate_partitions(problem_data &pd) { // O(n*c)
    find_fact_c_1(pd.c);
    for (int i = 0; i < pd.n; i++) {
        pd.partitions[i] = calculate_partition(pd.defaults[i], pd.sigmas[i]);
    }
}

bool is_majority_winner(const vorder &V, UL p) { //O(n) assuming plurality, could be any other P-time election
    vector<int> counts(V[0].size(), 0);
    for (const auto & v : V) {
        counts[v[0]-1]++;
    }
    return counts[p-1] == *max_element(counts.begin(), counts.end());
}

void f(vorder &V, problem_data &pd, UL i) { // O(c+n)
    double r_i = pd.randoms[i];
    int w = floor(r_i*V.size());
    order Q;
    for (int j = 0; j < pd.c; j++) {
        Q.insert(Q.begin()+floor(r_i*Q.size()), j+1);
    }
    // I'c not sure it's smart to use the same r_i every time, but let's see what happens
    if (is_majority_winner(V, pd.p) && (r_i*pd.partitions[w] < exp(-1*pd.sigmas[i]*dist(Q, pd.defaults[w])))) {
        V[w] = Q;
    }
}

void F(vorder &V, problem_data &pd) { // O(|R|(c+n))
    // compute the independent set
    for (UL i = pd.randoms.size(); i >= 1; i--) {
        f(V, pd, i-1);
    }
}

// O(c*n) due to copying
void get_majority_starting(problem_data &pd, vorder &top, vorder &bottom) {
    order base(pd.c);
    for (int i = 1; i <= pd.c; i++) { // O(c)
        base[i-1] = i;
    }

    // create top
    for (int i = 0; i < pd.n; i++) { // O(n)
        top[i] = order(base.rbegin(), base.rend());
        top[i].erase(top[i].begin()+base.size()-pd.p); // alternatively, worst().end()-p?
        top[i].insert(top[i].begin(), pd.p);
    }

    // create bottom
    UL curr = 1, total = pd.n, nc = pd.n/pd.c + (pd.n%pd.c != 0); // might degenerate if c == 1
    while (true) { // O(n*...)
        cout << "total=" << total << " nc=" << nc << endl;
        for (int i = 0; i < min(nc, total); i++) {
            cout << "i=" << i << endl;
            bottom[nc*(curr-1)+i] = order(base.begin(), base.end());
            bottom[nc*(curr-1)+i].erase(bottom[i].begin()+curr-1);
            bottom[nc*(curr-1)+i].insert(bottom[i].begin(), curr);
        }
        curr++;
        if (total <= nc) {
            break;
        }
        total -= nc;
    }
    print_vorder(bottom);
    cout << "adjusting" << endl;
    UL gotten = pd.n%pd.c;
    if (pd.p == pd.c && gotten != 0) { // O(n/c), `gotten` is kind of a misnomer here
        for (UL i = gotten; i < nc; i++) {
            bottom[pd.n+i] = order(bottom.back());
        }
    }
    print_vorder(bottom);
}

vorder sample(problem_data & pd) { // depends on R, which is dynamic.
    vorder top(pd.n), bottom(pd.n);
    generate_partitions(pd); // O(c*n)
    get_majority_starting(pd, top, bottom); //O(c*n)
    cout << "starters ready" << endl;
    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0) ;
    cout << "random generators ready" << endl;
    while (true) {
        vorder A(top), B(bottom);
        const unsigned  long L = max(1ul, pd.randoms.size());
        cout << "L=" << L << endl;
        for (int i = 0; i < L; i++) { // O(|R|)
            pd.randoms.push_back(unif(engine));
        }
        cout << "application one" << endl;
        F(A, pd); // O(|R|(c+n))
        cout << "application two" << endl;
        F(B, pd); // O(|R|(c+n))
        cout << "applications done" << endl;
        if (A == B) return A;
    }
}

int main() {
    int n, c;
    cin >> c >> n;
    problem_data pd(n, c);
    cin >> pd.p;

    for (int i = 0; i < pd.n; i++) { // O(nm)
        for (int j = 0; j < pd.c; j++) {
            int vote; cin >> vote; // assume a full-order (no ties)
            pd.defaults[i].push_back(vote);
        }
        cin >> pd.sigmas[i];
    }
    double _e,_d;
    cin >> _e >> _d;
    sample(pd);

    return 0;
}

// I think I could run this algorithm and compare the results with regular Monte Carlo to have some experimental data too
