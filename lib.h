#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <unordered_map>

using namespace std;

// these new types will help make the code more readable, and easier to debug
typedef unsigned long Candidate;
typedef vector<Candidate> Vote;
typedef vector<Candidate> CandidateSet;
typedef vector<Vote> VoteSet;

// overload the << operator to print vote set - used for debugging
ostream &operator<<(ostream &out, const VoteSet &votes) {
    unsigned long n = votes.size();
    out << ">>>>> START Vote Set with" << n << " votes <<<<<" << endl;
    for (unsigned long i = 0; i < n; i++) {
        const Vote &vote = votes[i];
        cout << i+1 << ": ";
        if (vote.empty()) {
            out << "--empty vote--";
        }
        for (Candidate candidate : vote) { // no need for else case, as loop never executes
            out << candidate << " ";
        }
        out << endl;
    }
    out << ">>>>> END Vote Set <<<<<" << endl;
    return out;
}

// keep a struct of all the inputs - easier to pass around to functions
// "Enhanced" because in COMSOC, "Election" refer to tuple of candidates and votes over those candidates
// As implemented, this does not allow for "masking" (as in the sense of control), but this can be done easily
//   by introducing a member to hold the candidate names, and turning "central_rankings" and "dispersions" to 
//   unordered_maps (and of course changing the type of p as needed)
struct EnhancedElection {
    
    private: // hide away the ugliness of the randomness being tied to the struct; used to make replication easier
    default_random_engine engine; // Needed to seed the number generator
    uniform_real_distribution<double> unif; // Uniform real number generator
    public:
    unsigned long   p /* distinguished candidate */, 
                    n /* num_voters*/, 
                    c/* num_candidates*/;
    vector<Vote> central_rankings; // reference ranking parameters for Mallows Model
    vector<double> dispersions; // dispersion parameters (often denoted by sigma) for Mallows Model
    double delta, epsilon; // confidence and precision parameters

    EnhancedElection() : engine(), unif(0.0, 1.0)  {
        auto seed = chrono::system_clock::now().time_since_epoch().count();
        cout << "SEED = " << seed << endl;
        engine = default_random_engine(seed);
        unif = uniform_real_distribution<double>(0.0, 1.0);

        cin >> c >> n;
        dispersions = vector<double>(n);
        central_rankings = vector<Vote>(n);
        cin >> p;

        for (unsigned long i = 0; i < n; i++) { // O(nm)
            for (unsigned long j = 0; j < c; j++) {
                unsigned long vote; 
                cin >> vote; // assume a full-order (no ties)
                central_rankings[i].push_back(vote);
            }
            cin >> dispersions[i];
        }
        cin >> epsilon >> delta;
    }

    // the actual function that should be used to obtain a random number
    double getRand() {
        return unif(engine);
    }

    ~EnhancedElection() = default;

};

// Checks if candidate p majority wins in votes
bool is_majority_winner(const VoteSet &votes, unsigned long numCandidates, Candidate p) { //O(n+c)
    unsigned long n = votes.size();
    if (numCandidates == 0) {
        cerr << "No candidates were provided" << endl;
        exit(-1);
    }
    // p cannot win uniquely if there are no votes, as majority ask for *more* than half of the votes, i.e., > 0
    if (n == 0) {
        return false;
    }
    
    // if there is at least one vote and p is the only candidate, then it will win
    if (numCandidates == 1) {
        return true;
    }

    // use a map to keep track of the scores of candidates; not using vector as it could be sparse
    unordered_map<Candidate, unsigned long> counts;
    for (const Vote &v : votes) { // count the votes each candidate gets
        if (counts.find(p) == counts.end()) { 
            counts[v.front()] = 1; 
        }
        else {
            counts[v.front()]++;  
        }
    }
    // Only one candidate can pass the threshold, so p passes iff no one else passes
    return counts[p] > floor(n/2); // use floor to avoid numerical instability
}