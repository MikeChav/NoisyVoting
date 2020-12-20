#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

#define UL unsigned long
typedef vector<UL> order;
#define _order const order &
typedef vector<order> vorder;
#define _vorder const vorder &

ostream &operator<<(ostream &out, _vorder V) {
    out << "START PEEK" << endl;
    for (const auto& x : V) {
        if (x.empty()) out << "--empty--";
        for (auto y : x) {
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
    default_random_engine engine;
    uniform_real_distribution<double> unif;
    double delta, epsilon;

    problem_data() : engine(chrono::system_clock::now().time_since_epoch().count()), unif(0.0, 1.0)  {
        cin >> c >> n;
        sigmas = vector<double>(n);
        defaults = vector<order>(n);
        cin >> p;

        for (UL i = 0; i < n; i++) { // O(nm)
            for (UL j = 0; j < c; j++) {
                UL vote; cin >> vote; // assume a full-order (no ties)
                defaults[i].push_back(vote);
            }
            cin >> sigmas[i]; // todo will need to assume these probabilities are > 0. If = 0, we can remove those voters
            // and adapt the threshold for p to win
        }
        cin >> epsilon >> delta;
    }

    ~problem_data() = default;

};

bool is_majority_winner(_vorder V, UL p) { //O(n+c)
    vector<UL> counts(V[0].size(), 0);
    for (_order v : V) {
        counts[v.front()-1]++;
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

inline order estimate_sample(problem_data &pd, UL w) { // O(c^2)
    double denom = 0.0;
    order sample;
    for (UL i = 0; i < pd.c; i++) {
        denom += pow(pd.sigmas[w], i);
        sample.insert(sample.begin()+get_insert_location(i, pd.sigmas[w], pd.unif(pd.engine)*denom), pd.defaults[w][i]);
    }
    return sample;
}

double monte_carlo(problem_data &pd, UL N) {
    UL count = 0;
    for (UL j = 0; j < N; j++) { // O(N(c^2+n))
        vector<order> sample(pd.n);
        for (UL i = 0; i < pd.n; i++) { // O(nc^2)
            sample[i] = estimate_sample(pd, i); //O(c^2)
        }
        count += is_majority_winner(sample, pd.p); //O(n+c)
    }
    return (double)count/(double)N;
}

UL sample_lll(problem_data &pd, vector<UL> &counters) { // O(Rnc^2(n+c))
    vorder V(pd.n);
    generate(V.begin(), V.end(), [&pd, &counters]() { // O(n)
        static UL i = 0;
        auto sample = estimate_sample(pd, i++ % pd.n);
        if (sample.front() == pd.p) counters[i]++;
        return sample;
    });

    UL R = 1;
//    UL R = V.size();
    while (!is_majority_winner(V, pd.p)) { // O(Rnc^2(n+c))
        for (UL i = 0; i < pd.n; i++) { // O(nc^2)
            if (V[i].front() != pd.p) {
                V[i] = estimate_sample(pd, i); // O(c^2)
                if (V[i].front() == pd.p) counters[i]++;
                R++;
            }
        }
//        R++;
    }
    return R;
}

double lll(problem_data &pd, UL N) {
    double estimate = 0.0;
    vector<UL> counters(pd.c);
    for (UL i = 0; i < N; i++) {// O(NRnc^2(n+c))
        auto x= pow(sample_lll(pd, counters), -1);
//        cout << x << endl;
        estimate += x;
//        estimate += pow(sample_lll(pd), -1); // O(Rnc^2(n+c))
    }

    return (double) estimate/(double)N;
}

int main() {
    problem_data pd;
    UL N = ceil((double)(pd.n*pd.n)*(-1*log(pd.delta))/(pd.epsilon*pd.epsilon));
    cout << N << endl;
    cout << "montecarlo=" << monte_carlo(pd, N) << endl;
    cout << "LLL=" << lll(pd, N) << endl;
    return 0;
}