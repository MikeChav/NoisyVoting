#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

typedef vector<int> order;
#define _order const order &


// todo make this into kendall-tau and adapt sampling accordingly
long dist(_order pi, _order pi_0) { //O(|pi|) = O(m), can be any poly(m) rule
    long total = 0;
    for (int i = 0; i < pi.size(); i++) {
        total += abs(pi[i]-pi_0[i]);
    }
    return total;
}

int get_insert_location(int i, double dispersion, double r) { // <<-- the bug must be here // O(m)
    double total = 0.0;
    for (int j = 0; j < i; j++) {
        total += pow(dispersion, i-j);
        if (r <= total) {
            return j;
        }
    }
    return i;  // under assumption that no placement was made due to numerical instability, just add at the end
}

order estimate_sample(_order pi, double dispersion, default_random_engine &engine, uniform_real_distribution<double> &unif) { // O(m^2)
    int m = pi.size(); double denom = 0.0;
    order sample;
    for (int i = 0; i < m; i++) {
        denom += pow(dispersion, i);
        sample.insert(sample.begin()+get_insert_location(i, dispersion, unif(engine)*denom), pi[i]);
    }
    return sample;
}

vector<int> evaluate_winners(const vector<order> &V) { //O(n) assuming plurality, could be any other P-time election
    vector<int> counts(V[0].size(), 0);
    for (const auto & v : V) {
        counts[v[0]-1]++;
    }
    vector<int> winners = {0};
    for (int i = 1; i < counts.size(); i++) {
        if (counts[i] > counts[winners[0]]) { // allowing for multiple winners
            winners.clear();
            winners.push_back(i);
        }
        else if (counts[i] == counts[winners[0]]) { winners.push_back(i);}
    }
    return winners;
}

int main() { //O((n/e)^2 log(1/delta) (m^2*n + 2n))
    int n /* num_voters */, m /* num_candidates */, p /* distinguised candidate */;
    long N, C = 0;
    double eps, delta;
    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0);
    cin >> m >> n;

    // in theory this variable is not needed, but I'm trying to get the whole distribution from the simulation
    vector<int> results(m, 0);
    vector<double> dispersions(n);
    vector<order> V(n); // V[i][j] indicates voter (i+1)'s (j+1)'s favorite candidate

    for (int i = 0; i < n; i++) { // O(nm)
        for (int j = 0; j < m; j++) {
            int vote; cin >> vote; // assume a full-order (no ties)
            V[i].push_back(vote);
        }
        cin >> dispersions[i];
    }
    cin >> p >> eps >> delta;
    N = n*n*log(1.0/delta)/(eps*eps); // todo reorder for optimality

    for (int j = 0; j < N; j++) { // O(Nm^2)
        vector<order> sample(n);
        for (int i = 0; i < n; i++) {
//            sample[i] = get_sample(distributions[i], unif(engine), engine, unif); //O(m!)
            sample[i] = estimate_sample(V[i], dispersions[i], engine, unif); //O(m^2)
        }
        auto winners = evaluate_winners(sample); // poly(n, m)
        for (auto w : winners) results[w]++; // O(m)
    }
    for (int i = 0; i < m; i++) cout <<  results[i] << " "; cout << endl;
    for (int i = 0; i < m; i++) {
        cout << "Probability " << i + 1 << " wins is: " << double(results[i]) / double(N) << endl;
    }

    return 0;
}
