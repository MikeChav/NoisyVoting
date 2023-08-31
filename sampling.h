#include "lib.h"

// This is used in RIM (Repeated Insertion Method) sampling, which is a polynomial-time way of sampling from a Mallow Model
unsigned long get_insert_location(unsigned long i, double sigma, double r) { // O(c)
    // insert at location j with probability sigma^(i-j)/*(\sum_{k=0}^{i-1} sigma^k)
    // the fraction is scaled by the denominator early on, hence the introduction of r
    double total = 0.0;
    for (unsigned long j = 1; j <= i; j++) {
        total += pow(sigma, i - j);
        if (r <= total) {
            return j-1; // RIM is defined as 1-indexed, but need to adjust here
        }
    }
    return i;  // under assumption that no placement was made due to numerical instability, just add at the end
}


// sample from the mallows model using RIM (Repeated Insertion Method) sampling, which is a polynomial-time way of sampling from a Mallow Model
inline Vote estimate_sample(EnhancedElection &election, unsigned long w) { // O(c^2)
    double denom = 0.0; // keeps track of \sum_{k=0}^{k=c} \sigma^k throughout the method
    Vote sample; // start with empty vote
    double sigma = election.dispersions[w];
    for (unsigned long i = 0; i < election.c; i++) {
        denom += pow(sigma, i);
        // selection location to insert a candidate in the vote (order) probabilistically
        unsigned long j = get_insert_location(i, sigma, election.getRand()*denom);
        sample.insert(sample.begin()+j, election.central_rankings[w][i]);
    }
    return sample;
}

// implementation for the monte carlo method
// N is the number of samples to take
double monte_carlo(EnhancedElection &election, unsigned long N) {
    unsigned long count = 0;
    for (unsigned long j = 0; j < N; j++) { // O(N(c^2+n))
        VoteSet sample(election.n);
        // generate a random vote set
        generate(sample.begin(), sample.end(), [&election]() {
            static unsigned long i = 0;
            return estimate_sample(election, i++);}); // O(n)
        // increment a counter if p wins in the vote set
        count += is_majority_winner(sample, election.p, election.c); //O(n+c)
    }
    // return the fraction of samples in which p won
    return (double)count/(double)N;
}

// implementation based on the Moser--Tardos method (Algorithmic Lovasz Local Lemma)
pair<VoteSet, unsigned long> sample_lll(EnhancedElection &election) { // O(Rnc^2(n+c))
    VoteSet V(election.n);
    // generate a random vote set
    generate(V.begin(), V.end(), [&election]() { // O(n)
        static unsigned long i = 0;
        return estimate_sample(election, i++);
    });

    unsigned long R = 1; // count the number of votes generated
    // while there is bad event (i.e. p is not winning); technically, Moser--Tardos requires more than that
    //   as it only terminates when all bad events are gone, but we terminate when enough bad events are gone
    //   and in practice, this seems to yield acceptable results
    while (!is_majority_winner(V, election.p, election.c)) { // O(Rnc^2(n+c))
        for (unsigned long i = 0; i < election.n; i++) { // O(nc^2)
            if (V[i].front() != election.p) {
                // resample the variables of the event i.e. the votes where p was not ranked first
                V[i] = estimate_sample(election, i); // O(c^2)
            }
        }
        R++;
    }
    return make_pair(V, R);
}

double lll(EnhancedElection &election, unsigned long N) {
    double estimate = 0.0;
    for (unsigned long i = 0; i < N; i++) {// O(NRnc^2(n+c))
        // count the number of tries needed to get the sample, and use the inverse as the probability estimate
        // only need the count here, not the actual vote set
        estimate += pow(sample_lll(election).second, -1); // O(Rnc^2(n+c))
    }
    // return the average of the estimate
    return (double) estimate/(double)N;
}