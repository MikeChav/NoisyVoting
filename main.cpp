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

uint64_t fact_m_1 = 0; // todo replace with gmp int


struct problem_data {
    UL p, n /* num_voters*/, m/* num_candidates*/;
    vorder defaults;
    vector<double> sigmas;
    vector<double> randoms;
    vector<double> partitions;

    problem_data() : p(0), n(0), m(0) {}
};

double dist(_order pi, _order pi_0) { //O(|pi|) = O(m)
    // in this implementation, we consider the following weight vector: <1, 0, ..., 0>
    // Thus the implementation reduces simply to:
    return abs(pi[0] - pi_0[0]);
}

void find_fact_m_1(UL m) { // O(m)
    if (fact_m_1 == 0) {
        fact_m_1 = 1;
        for (int i = 2; i < m; i++) {
            fact_m_1 *= i;
        }
    }
}

double calculate_partition(_order pi_0, double sigma) { // O(m*n)
    // this function is very easy to implement in O(m) time since the distance function only returns one of m value
    // note this is done per voter
    double Z = 0.0;
    for (int i = 1; i <= pi_0.size(); i++) {
        Z += exp(-1*sigma*abs(i-pi_0[0]));
    }
    return Z*fact_m_1;
}

void generate_partitions(problem_data &pd) {
    find_fact_m_1(pd.m);
    for (int i = 0; i < pd.n; i++) {
        pd.partitions[i] = calculate_partition(pd.defaults[i], pd.sigmas[i]);
    }
}

bool is_majority_winner(const vector<order> &V, UL p) { //O(n) assuming plurality, could be any other P-time election
    vector<int> counts(V[0].size(), 0);
    for (const auto & v : V) {
        counts[v[0]-1]++;
    }
    return counts[p-1] == *max_element(counts.begin(), counts.end());
}

void f(vorder &V, problem_data &pd, UL i) {
    double r_i = pd.randoms[i];
    int w = floor(r_i*V.size());
    order Q;
    for (int j = 0; j < V[0].size(); j++) {
        Q.insert(Q.begin()+floor(r_i*Q.size()), j);
    }
    order backup = V[w];
    V[w] = Q;

    // I'm not sure it's smart to use the same r_i every time, but let's see what happens
    if (is_majority_winner(V, pd.p) && (r_i*pd.partitions[w] < exp(-1*pd.sigmas[i]*dist(Q, pd.defaults[w])))) {
        // nothing to do, V has been updated.
    }
    else {
        V[w] = backup;
    }
}

void F(vorder &V, problem_data &pd) {
    // compute the independent set
    for (UL i = pd.randoms.size(); i >= 1; i--) {
        f(V, pd, i);
    }
}


order get_best(UL p, const order &base) {
    order best(base.begin(), base.end());
    best.erase(base.begin()+p-1);
    best.insert(best.begin(), p);
    return best;
}

order get_worst(UL p, const order &base) {
    order worst(base.rbegin(), base.rend());
    worst.erase(worst.begin()+base.size()-p); // alternatively, worst().end()-p?
    worst.insert(worst.begin(), p);
    return worst;
}

// O(m+n) amortized (from C++ documentation, push_back is amortized constant time
void get_majority_starting(problem_data &pd, vorder &top, vorder &bottom) {
    order best, worst, good_bad, base(pd.n);
    for (int i = 1; i <= pd.m; i++) { // O(m)
        base[i-1] = i;
    }

    // create top
    order worst_p = get_worst(pd.p, base); // O(m)
    for (int i = 0; i < pd.n; i++) { // O(n)
        top.push_back(worst_p);
    }

    // create bottom
    UL curr = 1, total = pd.n, nc = pd.n/pd.m;
    while (total != 0) { // O(n)
        order curr_best = get_best(curr++, base);
        for (int i = 0; i < min(nc, total); i++) {
            bottom.push_back(curr_best);
        }
        total -= nc;
    }
    UL gotten = pd.n%pd.m;
    if (pd.p == pd.m && gotten != 0) { // O(n/m), `gotten` is kind of a misnomer here
        for (UL i = gotten; i < nc; i++) {
            bottom[pd.n+i] = bottom.back();
        }
    }
}

vorder sample(problem_data & pd) {
    vorder top, bottom;
    generate_partitions(pd);
    get_majority_starting(pd, top, bottom);
    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0) ;
    while (true) {
        vorder A(top), B(bottom);
        const unsigned  long L = max(1ul, pd.randoms.size());
        for (int i = 0; i < L; i++) {
            pd.randoms.push_back(unif(engine));
        }
        F(A, pd);
        F(B, pd);
        if (A==B) return A;
    }
}

int main() { //O((n/e)^2 log(1/delta) (m^2*n + 2n))
    problem_data pd;
    cin >> pd.m >> pd.n >> pd.p;

    for (int i = 0; i < pd.n; i++) { // O(nm)
        for (int j = 0; j < pd.m; j++) {
            int vote; cin >> vote; // assume a full-order (no ties)
            pd.defaults[i].push_back(vote);
        }
        cin >> pd.sigmas[i];
    }

    sample(pd);

    return 0;
}

// I think I could run this algorithm and compare the results with regular Monte Carlo to have some experimental data too
