#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

typedef vector<int> order;
#define _order const order &
#define vorder vector<order>

// todo create a struct with all election info - easier to pass around

// todo make this into kendall-tau and adapt sampling accordingly
long dist(_order pi, _order pi_0) { //O(|pi|) = O(m), can be any poly(m) rule
    long total = 0;
    for (int i = 0; i < pi.size(); i++) {
        total += abs(pi[i]-pi_0[i]);
    }
    return total;
}

bool is_majority_winner(const vector<order> &V, int p) { //O(n) assuming plurality, could be any other P-time election
    vector<int> counts(V[0].size(), 0);
    for (const auto & v : V) {
        counts[v[0]-1]++;
    }
    int highest_score = *max_element(counts.begin(), counts.end());
    return counts[p-1] == highest_score;
}

void f(vorder &V, double r_i, double sigma_i, int p) {
    int w = floor(r_i*V.size());
    order Q;
    for (int i = 0; i < V[0].size(); i++) {
        Q.insert(Q.begin()+floor(r_i*Q.size()), i);
    }
    order backup = V[w];
    V[w] = Q;
    if (is_majority_winner(V, p) && ()) {
        //
    }
    else {
        V[w] = backup;
    }
}

void F(vorder &V, const vector<double> &R, const vector<double> &sigma, int p) {
    // compute the independent set
    for (unsigned long i = R.size(); i >= 1; i--) {
        f(V, R[i-1], sigma[i-1], p);
    }
}


/*
 * This
 *
 * idea for poset:
 *    x <= y iff #votes_x(p) < #votes_y(p) or (#votes_x(p) = #votes_y(p) and tail(x_i) is not better ordered than tail(y_i))
 *              formally H(tail(x_i), ordered(tail(x_i))) < H(tail(y_i), ordered(tail(y_i)))
 *                      but is ordered(tail(y)) == ordered(tail(x))
 * This is a poset - is this useful only in the A/B construction?
 *          I'm having trouble constructing a minimal value here
 */
/*
 Bottom: p
 Top: Everyone puts p on top but the rest is in order
 Bottom: everyone gets as equal votes as possible ... that's an issue now if n = 5, c = 3, who ends with only 1 vote?
 Ok... This relation looks like a poset, but there's an issue with the final ordering (^). My definition is too vague.
 this requires a condition on the rows too i.e. [[2,...], [2, ...], [1, ...], [1, ...], [3,...]] is preferred to ANY
 other vote set V where #votes_p(V) = 2 and p = 2.
 */
void get_majority_starting(int p, int nv, int nc, vorder &A, vorder &B) {
    order best, worst, good_bad;
    best.push_back(p);
    good_bad.push_back(p);
    worst.push_back(nc);
    int adj = 1;
    for (int i = 2; i <= nc; i++) {
        worst.push_back(nc-i+1);
        if (i == p) {
            adj = 0;
            continue;
        }
        best.push_back(i-adj);
        good_bad.push_back(nc-i+1-adj);
    }

    int L = (nv+1)/2;
    for (int i = 1; i <= nv; i++) {
        A.push_back(best);
        if (i <= L) {
            B.push_back(good_bad);
        }
        else {
            B.push_back(worst);
        }
    }

}

vorder sample(int p, const vector<double> &sigma, int c) {
    vorder start_1, start_2;
    get_majority_starting(p, sigma.size(), c, start_1, start_2);
    vector<double> R;
    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0) ;
    while (true) {
        vorder A(start_1), B(start_2);
        R.push_back(unif(engine));
        F(A, R, sigma, p);
        F(B, R, sigma, p);
        if (A==B) return A;
    }
}

//int main() { //O((n/e)^2 log(1/delta) (m^2*n + 2n))
//    int n /* num_voters */, m /* num_candidates */, p /* distinguised candidate */;
//    long N, C = 0;
//    double eps, delta;
//    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
//    uniform_real_distribution<double> unif(0.0, 1.0);
//    cin >> m >> n;
//
//    // in theory this variable is not needed, but I'm trying to get the whole distribution from the simulation
//	// using input1, the probabilities don't sum to 1. It's because we have non-unique winners
//	// todo figure out if probabilities need to be adjusted based on ^---
//	vector<int> results(m, 0);
//    vector<double> dispersions(n);
//    vector<order> V(n); // V[i][j] indicates voter (i+1)'s (j+1)'s favorite candidate
//
//    for (int i = 0; i < n; i++) { // O(nm)
//        for (int j = 0; j < m; j++) {
//            int vote; cin >> vote; // assume a full-order (no ties)
//            V[i].push_back(vote);
//        }
//        cin >> dispersions[i];
//    }
//    cin >> p >> eps >> delta;
//    N = n*n*log(1.0/delta)/(eps*eps); // todo reorder for optimality
//
//    for (int j = 0; j < N; j++) { // O(Nm^2)
//        vector<order> sample(n);
//        for (int i = 0; i < n; i++) {
////            sample[i] = get_sample(distributions[i], unif(engine), engine, unif); //O(m!)
//            sample[i] = estimate_sample(V[i], dispersions[i], engine, unif); //O(m^2)
//        }
//        auto winners = evaluate_winners(sample); // poly(n, m)
//        for (auto w : winners) results[w]++; // O(m)
//    }
//    for (int i = 0; i < m; i++) cout <<  results[i] << " "; cout << endl;
//    for (int i = 0; i < m; i++) {
//        cout << "Probability " << i + 1 << " wins is: " << double(results[i]) / double(N) << endl;
//    }
//
//    return 0;
//}
