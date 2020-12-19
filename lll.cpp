#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

#define UL unsigned long
typedef vector<UL> order;
#define _order const order &

struct vote {
    UL voter;
    order prefs;

    vote() :voter(0) {}

    vote(UL n): prefs(n) {}

    bool operator==(const vote &rhs) const {
        return voter == rhs.voter && prefs == rhs.prefs;
    }

};

// voters are 0-indexed and candidates are 1-indexed
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
        for (UL i = 0; i < n; i++) {
            V[i].voter = i; // voters are stored 0-indexed
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

    bool operator==(const votes &rhs) const {
        return m == rhs.m && V == rhs.V;
    }

    friend ostream &operator<<(ostream &out, const votes &W);
};

ostream &operator<<(ostream &out, const votes &W) {
    out << "START PEEK" << endl;
    for (const auto& x : W.V) {
        if (x.prefs.empty()) out << "--empty--";
        for (auto y : x.prefs) {
            out << y << " ";
        }
        out << endl;
    }
    out << "END PEEK" << endl;
    return out;
}

struct problem_data {
    UL p, n /* num_voters*/, c/* num_candidates*/;
    vector<order> defaults;
    vector<double> sigmas;
    vector<double> randoms;
    default_random_engine engine;
    uniform_real_distribution<double> unif;

    problem_data(UL n, UL c) : p(0), n(n), c(c), engine(chrono::system_clock::now().time_since_epoch().count()), unif(0.0, 1.0) {
        sigmas = vector<double>(n);
        defaults = vector<order>(n);
    }
};

bool is_majority_winner(const votes &V, UL p) { //O(n) assuming plurality, could be any other P-time election
    vector<UL> counts(V[0].size(), 0);
    for (const vote &v : V.V) {
        counts[v.prefs.front()-1]++;
    }
    return counts[p-1] == *max_element(counts.begin(), counts.end());
}

UL get_insert_location(UL i, double dispersion, double r) { // O(c)
    double total = 0.0;
    for (UL j = 0; j < i; j++) {
        total += pow(dispersion, i-j);
        if (r <= total) {
            return j;
        }
    }
    return i;  // under assumption that no placement was made due to numerical instability, just add at the end
}

inline order estimate_sample(problem_data &pd, UL w) { // O(m^2)
    double denom = 0.0;
    order sample;
    for (UL i = 0; i < pd.c; i++) {
        denom += pow(pd.sigmas[w], i);
        sample.insert(sample.begin()+get_insert_location(i, pd.sigmas[w], pd.unif(pd.engine)*denom), pd.defaults[w][i]);
    }
    return sample;
}

void sample(problem_data &pd) { // O(c+n)
    votes V(pd.n);

    for (UL i = 0; i < pd.n; i++) {
        V.set(i, estimate_sample(pd, i));
    }
    UL R = 0;
    while (!is_majority_winner(V, pd.p)) {
        for (UL i = 0; i < pd.n; i++) {
            if (V[i].front() != pd.p) {
                R++;
                V.set(i, estimate_sample(pd, i)); // todo need to add the witness tree or something so we don't have repeats
            }
        }
    }
    cout << V << endl;
    cout << "Number of re-samples = " << R << endl;
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
        cin >> pd.sigmas[i]; // todo will need to assume these probabilities are > 0. If = 0, we can remove those voters
                             // and adapt the threshold for p to win
    }
    double _e,_d;
    cin >> _e >> _d;
    sample(pd);

    return 0;
}

// I think I could run this algorithm and compare the results with regular Monte Carlo to have some experimental data too
