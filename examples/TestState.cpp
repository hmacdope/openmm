#include <OpenMM.h>

#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace OpenMM;

Context* coreContext_ = nullptr;
Context* refContext_ = nullptr;


void testState(const State& state);

int main() {
    ifstream sys_inp("system.xml");
    System* system = XmlSerializer::deserialize<System>(sys_inp);
    ifstream integrator_inp("system.xml");
    Integrator* integrator_1 = XmlSerializer::deserialize<Integrator>(integrator_inp);
    Integrator* integrator_2 = XmlSerializer::deserialize<Integrator>(integrator_inp);
    ifstream state_inp("state.xml");
    State* state = XmlSerializer::deserialize<State>(state_inp);

    // read state
    state->getPositions();
    state->getForces();
    state->getVelocities();
    auto k_e = state->getKineticEnergy();
    auto pot_e = state->getPotentialEnergy();

    std::cout << "Potential energy: " << pot_e << "\n";
    std::cout << "Kinetic Energy: " << k_e << "\n";

    refContext_ = new Context(*system, *integrator_1, Platform::getPlatformByName("CPU"));
    coreContext_ = new Context(*system, *integrator_2, Platform::getPlatformByName("CUDA"));

    testState(*state);

    // Clean up memory
    delete state;
    delete integrator_1;
    delete integrator_2;
    delete system;
    delete coreContext_;
    delete refContext_;

    return 0;
}

void compareEnergies(const State& a, const State& b, double tolerance) {
    const double relativeTol = 1e-4;
    double potentialEnergyA = a.getPotentialEnergy();
    double potentialEnergyB = b.getPotentialEnergy();

    cout.precision(20);
    cout << "Potential Energy A: " << potentialEnergyA << endl;
    cout << "Potential Energy B: " << potentialEnergyB << endl;

    double diff = fabs(potentialEnergyA - potentialEnergyB);

    if (diff > tolerance && diff > relativeTol * fabs(potentialEnergyA))
        cout << "Potential energy error of " << diff << ", threshold of "
             << tolerance << '\n'
             << "Reference Potential Energy: " << potentialEnergyA
             << " | Given Potential Energy: " << potentialEnergyB;

    double kineticEnergyA = a.getKineticEnergy();
    double kineticEnergyB = b.getKineticEnergy();
    diff = fabs(kineticEnergyA - kineticEnergyB);

    if (diff > tolerance && diff > relativeTol * fabs(kineticEnergyA))
        cout << "Kinetic energy error of " << diff << ", threshold of "
             << tolerance << '\n'
             << "Reference Kinetic Energy: " << kineticEnergyA
             << " | Given Kinetic Energy: " << kineticEnergyB;
}

void testState(const State& state) {
    refContext_->setState(state); // toggle
    compareEnergies(refContext_->getState(State::Forces | State::Energy), state, 10);
}
