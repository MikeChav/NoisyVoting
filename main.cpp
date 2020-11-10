#include <iostream>
#include <vector>
#include <map>
#include <numeric>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

typedef vector<int> order;
typedef map<order, double> distribution;

#define _order const order &
#define _distribution const distribution &


long dist(_order pi, _order pi_0) { //O(m)
    long total = 0;
    for (int i = 0; i < pi.size(); i++) {
        total += abs(pi[i]-pi_0[i]);
    }
    return total;
}

distribution compute_distribution(double disp, _order pi_0) { //O(m!)
    // todo potentially improve by running this m! factorial step once
    vector<int> pi(pi_0.size());
    distribution  distrib;
    iota(pi.begin(), pi.end(), 1);
    double total = 0.0;
    do {
        double p = pow(exp(-disp), dist(pi, pi_0)); // todo check if best way to do this
        total += p;
        distrib[pi] = p;
    } while (next_permutation(pi.begin(), pi.end()));
    for (auto & di : distrib) {
        di.second /= total;
    }
    return distrib;
}

order get_sample(_distribution pi, double r) { // O(m!)
    double total = 0.0;
    order last;
    for (const auto& p : pi) { // making use of the fact that map is ordered
        total += p.second;
        last = p.first;
        if (r <= total) return last;
    }
    return last; // assuming some slight accuracy error happened and we're in the range for the last number
}

vector<int> evaluate_winner(const vector<order> &V) { //O(n) assuming plurality
    vector<int> counts(V.size(), 0);
    for (const auto & v : V) {
        counts[v[0]-1]++;
    }
    vector<int> winners = {0};
    for (int i = 1; i < V.size(); i++) {
        if (counts[i] > counts[winners[0]]) { // allowing for multiple winners
            winners.clear();
            winners.push_back(i);
        }
        else if (counts[i] == counts[winners[0]]) { winners.push_back(i);}
    }
    return winners;
}

int main() { //O((n/e)^2 log(1/delta) (m!*n + 2n))
    int n /* num_voters */, m /* num_candidates */, p /* distinguised candidate */;
    long N, C = 0;
    double eps, delta;
    cin >> m >> n;
    vector<double> dispersions(n);
    vector<order> V(n); // V[i][j] indicates voter i's (j+1)'s favorite candidate
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int vote; cin >> vote;
            V[i].push_back(vote);
        }
        cin >> dispersions[i];
    }
    cin >> p >> eps >> delta;

    N = (n*n*log(1.0/delta))/(eps*eps); // todo reorder for optimality

    vector<distribution> distributions(n);
    for (int i = 0; i < distributions.size(); i++) { // O(m!)
        distributions[i] = compute_distribution(dispersions[i], V[i]);
    }

    default_random_engine engine(chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> unif(0.0, 1.0);
    vector<int> results(m, 0);
    for (int j = 0; j < N; j++) {
        vector<order> sample(n);
        for (int i = 0; i < n; i++) {
            sample[i] = get_sample(distributions[i], unif(engine)); //O(m!)
        }
        auto winners = evaluate_winner(sample);
        for (auto w : winners) results[w-1]++;

    }
    for (int i = 0; i < m; i++)
        cout << "Probability " << i+1 << " wins is: "<< double(results[i])/double(N) << endl;
    return 0;
}
