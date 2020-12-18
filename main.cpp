#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

#define UL unsigned long
typedef vector<UL> order;
#define _order const order &
#define vorder vector<order> // [i][j] indicates voter (i+1)'s (j+1)'s favorite candidate

void print_vorder(const vorder &V) {
    cout << "START PEEK" << endl;
    for (const auto& x : V) {
        if (x.empty()) cout << "--empty--";
        for (auto y : x) {
            cout << y << " ";
        }
        cout << endl;
    }
    cout << "END PEEK" << endl;
}

struct vote {
    UL voter;
    order prefs;

    vote(UL n) {
        prefs = order(n);
    }

};

struct votes {
    vector<UL> m;
    vector<vote> V;

    void reset_map() {
        for (UL i = 0; i < V.size(); i++) {
            m[V[i].voter] = i;
        }
    }

    votes(UL n) {
        m = vector<UL>(n);
        V = vector<vote>(n);
        for (int i = 0; i < n; i++) {
            V[i].voter = i+1; // voters are stored 1-indexed
        }
        reset_map();
    }

    void sort() { // O(nlogn)
        std::sort(V.begin(), V.end(),
                  [](const vote &a, const vote &b) {
                        return a.prefs.front() < b.prefs.front();
        });
        reset_map();
    }

    order operator[](UL i) const { // 0-indexed
        return V[m[i]].prefs;
    }

    order &operator[](UL i) { // 0-indexed
        return V[m[i]].prefs;
    }

    void set(UL w, const order &Q) {
        V[m[w]].prefs = order(Q);
    }

    void replace(UL w, const order &Q) {
        set(w, Q);
        sort();
    }
};

struct problem_data {
    UL p, n /* num_voters*/, c/* num_candidates*/;
    vector<order> defaults;
    vector<double> sigmas;
    vector<double> randoms;

    problem_data(UL n, UL c) : p(0), n(n), c(c) {
        sigmas = vector<double>(n);
        defaults = vector<order>(n);
        for (UL i = 0; i < n; i++) {
            defaults[i] = order(c);
        }
    }
};

inline double dist(_order pi, _order pi_0) { //O(|pi|) = O(c)
    // in this implementation, we consider the following weight vector: <1, 0, ..., 0>
    // Thus the implementation reduces simply to:
    return (double) max(pi[0], pi_0[0]) - min(pi[0], pi_0[0]);
}

bool is_majority_winner(const votes &V, UL p) { //O(n) assuming plurality, could be any other P-time election
    vector<UL> counts(V[0].size(), 0);
    for (const auto & v : V.V) {
        counts[v.prefs.front()-1]++;
    }
    return counts[p-1] == *max_element(counts.begin(), counts.end());
}

void f(votes &V, problem_data &pd, double r_i) { // O(c+n)
    UL w = floor(r_i*pd.n); // w.p. 1/n
    order Q; // w.p. H_n
    for (UL j = 0; j < pd.c; j++) {
        Q.insert(Q.begin()+floor(r_i*Q.size()), j+1);
    }
    // I'm not sure it's smart to use the same r_i every time, but let's see what happens

    double alpha = exp(-1*pd.sigmas[w]*dist(pd.defaults[w], Q));
    double rho = exp(-1*pd.sigmas[w]*dist(pd.defaults[w], V[w]));
    order backup(V[w]);
    V.replace(w, Q); // O(nlogn)
    if (is_majority_winner(V, pd.p)) {
//        cout << pd.p << " majority wins in " << endl;
//        print_vorder(V);
        if (r_i <= alpha/(rho+alpha)) {

        }
        else {
            V.replace(w, backup); // O(nlogn)
        }
    }
    else {
        V.replace(w, backup); // O(nlogn)
    }
}

void F(votes &V, problem_data &pd) { // O(|R|(c+n))
    for (UL i = pd.randoms.size(); i >= 1; i--) {
        f(V, pd, pd.randoms[i-1]);
    }
}

// O(c*n) due to copying
void get_majority_starting(problem_data &pd, votes &top, votes &bottom) {
    order base(pd.c);
    iota(base.begin(), base.end(), 1);  // O(c)

    // create bottom
    // allocate equal vote chunks to each candidate while votes are available
    UL curr = 1, total = pd.n, nc = pd.n/pd.c + (pd.n%pd.c != 0); // might degenerate if c == 1
    while (true) { // O(n*...)
        for (UL i = 0; i < min(nc, total); i++) {
            UL k = nc*(curr-1)+i;
            bottom.set(k, order(base.begin(), base.end()));
            bottom[k].erase(bottom[k].begin()+curr-1);
            bottom[k].insert(bottom[k].begin(), curr);
        }
        curr++;
        if (total <= nc) {
            break;
        }
        total -= nc;
    }
    // the last candidate might be underserved if we want them to win, so an adjustment is needed
    UL gotten = pd.n%nc;
    if (pd.p == pd.c && gotten != 0) { // O(n/c), `gotten` is kind of a misnomer here
        for (UL i = gotten; i < nc; i++) {
            bottom.set(pd.n-i-1, order(bottom.V.back().prefs));
        }
    }

    // create top
}

votes sample(problem_data & pd) { // depends on R, which is dynamic.
    votes top(pd.n), bottom(pd.n);
//    generate_partitions(pd); // O(c*n)
    get_majority_starting(pd, top, bottom); //O(c*n)
    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0) ;
    while (true) {
        votes A(top), B(bottom);
        const unsigned  long L = max(1ul, pd.randoms.size());
        for (UL i = 0; i < L; i++) { // O(|R|)
            pd.randoms.push_back(unif(engine));
        }
        cout << "L=" << pd.randoms.size() << endl;

        F(A, pd); // O(|R|(c+n))
        F(B, pd); // O(|R|(c+n))
        if (A == B) return A;
    }
}

int main() {
    UL n, c;
    cin >> c >> n;
    problem_data pd(n, c);
    cin >> pd.p;

    for (UL i = 0; i < pd.n; i++) { // O(nm)
        for (UL j = 0; j < pd.c; j++) {
            UL vote; cin >> vote; // assume a full-order (no ties)
            pd.defaults[i].push_back(vote);
        }
        cin >> pd.sigmas[i];
    }
    double _e,_d;
    cin >> _e >> _d;
    print_vorder(sample(pd));

    return 0;
}

// I think I could run this algorithm and compare the results with regular Monte Carlo to have some experimental data too
