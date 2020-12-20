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

// overload the << operator to print vote set - used for debugging
ostream &operator<<(ostream &out, _vorder V) {
    out << "START Vote Set" << endl;
    for (const auto& x : V) {
        if (x.empty()) out << "--empty--";
        for (auto y : x) {
            out << y << " ";
        }
        out << endl;
    }
    out << "END Vote Set" << endl;
    return out;
}

// keep a struct of all the inputs - easier to pass around to functions
struct problem_data {
    UL p /* distinguished candidate */, n /* num_voters*/, c/* num_candidates*/;
    vector<order> defaults; // reference ranking parameters for Mallows Model
    vector<double> sigmas; // dispersion parameters for Mallows Model
    default_random_engine engine; // Needed to seed the number generator
    uniform_real_distribution<double> unif; // Random real number generator
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
            cin >> sigmas[i];
        }
        cin >> epsilon >> delta;
    }

    ~problem_data() = default;

};

// Checks if candidate p majority wins in vote set V
bool is_majority_winner(_vorder V, UL p) { //O(n+c)
    vector<UL> counts(V[0].size(), 0);
    for (_order v : V) {
        counts[v.front()-1]++; // count the votes each candidate gets
    }
    return counts[p-1] == *max_element(counts.begin(), counts.end()); // make sure no candidate has more votes than p
}

// This is used in RIM sampling
UL get_insert_location(UL i, double sigma, double r) { // O(c)
    double total = 0.0;
    for (UL j = 0; j < i; j++) {
        total += pow(sigma, i - j);
        if (r <= total) {
            return j;
        }
    }
    return i;  // under assumption that no placement was made due to numerical instability, just add at the end
}


// sample from the mallows model using the RIM method
inline order estimate_sample(problem_data &pd, UL w) { // O(c^2)
    double denom = 0.0;
    order sample; // start with empty vote
    for (UL i = 0; i < pd.c; i++) {
        denom += pow(pd.sigmas[w], i);
        // selection location to insert a candidate in the vote (order) probabilistically
        sample.insert(sample.begin()+get_insert_location(i, pd.sigmas[w], pd.unif(pd.engine)*denom), pd.defaults[w][i]);
    }
    return sample;
}

// implementation for the monte carlo method
double monte_carlo(problem_data &pd, UL N) {
    UL count = 0;
    for (UL j = 0; j < N; j++) { // O(N(c^2+n))
        vector<order> sample(pd.n);
        // generate a random vote set
        UL i = 0;
        generate(sample.begin(), sample.end(), [&pd, &i]() { return estimate_sample(pd, i++);}); // O(n)
        // increment a counter is p wins in the vote set
        count += is_majority_winner(sample, pd.p); //O(n+c)
    }
    // return the fraction of samples in which p won
    return (double)count/(double)N;
}

// implementation for the Moser-Tardos method (algorithmic Lovasz Local Lemma)s
UL sample_lll(problem_data &pd) { // O(Rnc^2(n+c))
    vorder V(pd.n);
    // generate a random vote set
    generate(V.begin(), V.end(), [&pd]() { // O(n)
        static UL i = 0;
        return estimate_sample(pd, i++ % pd.n);
    });

    UL R = 1; // count the number of votes generated
    // while there is bad event (i.e. p is not winning)
    while (!is_majority_winner(V, pd.p)) { // O(Rnc^2(n+c))
        for (UL i = 0; i < pd.n; i++) { // O(nc^2)
            if (V[i].front() != pd.p) {
                // resample the variables of the event i.e. the votes where p was not ranked first
                V[i] = estimate_sample(pd, i); // O(c^2)
            }
        }
        R++;
    }
    return R;
}

double lll(problem_data &pd, UL N) {
    double estimate = 0.0;
    for (UL i = 0; i < N; i++) {// O(NRnc^2(n+c))
        // count the number of tries needed to get the sample, and use the inverse as the probability estimate
        estimate += pow(sample_lll(pd), -1); // O(Rnc^2(n+c))
    }
    // return the average of the estimate
    return (double) estimate/(double)N;
}

int main() {
    problem_data pd; // the constructor reads data from stdin
    UL N = ceil((double)(pd.n*pd.n)*(-1*log(pd.delta))/(pd.epsilon*pd.epsilon));
    cout << "montecarlo=" << monte_carlo(pd, N) << endl;
    cout << "LLL=" << lll(pd, N) << endl;
    return 0;
}