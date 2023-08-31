#include "sampling.h"

int main() {
    EnhancedElection election; // the constructor reads data from stdin
    unsigned long N = ceil((double)pow(election.n, 2)*(-1*log(election.delta))/pow(election.epsilon, 2));
    cout << "montecarlo=" << monte_carlo(election, N) << endl;
    cout << "LLL=" << lll(election, N) << endl;
    return 0;
}