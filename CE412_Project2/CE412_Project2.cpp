// CE412_Project2.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Bilge ÞEKER              20191701007
// Ferihan HACIALÝOÐLU      20191701043

using namespace std;

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <functional>
#include <string>

using namespace std;

// Define materials and products
class Material {
public:
    string name;
    Material(const string& name) : name(name) {}
};

class Product : public Material {
public:
    Product(const string& name) : Material(name) {}
};

// System Modeling: Resources
class Resource {
public:
    string name;
    bool isAvailable;

    Resource(const string& name) : name(name), isAvailable(true) {}

    void use() {
        isAvailable = false;
    }

    void release() {
        isAvailable = true;
    }
};

class Operator : public Resource {
public:
    Operator(const string& name) : Resource(name) {}
};

class Machine : public Resource {
public:
    Machine(const string& name) : Resource(name) {}
};

// Event management
class Event {
public:
    double time;
    function<void()> action;

    Event(double time, function<void()> action) : time(time), action(action) {}

    bool operator>(const Event& other) const {
        return time > other.time;
    }
};

class EventQueue {
private:
    priority_queue<Event, vector<Event>, greater<Event>> events;

public:
    void addEvent(double time, function<void()> action) {
        events.push(Event(time, action));
    }

    void processNextEvent(double& currentTime) {
        if (!events.empty()) {
            Event event = events.top();
            events.pop();
            currentTime = event.time;
            event.action();
        }
    }

    bool isEmpty() {
        return events.empty();
    }
};

class SimulationClock {
private:
    double currentTime;
    EventQueue eventQueue;

public:
    SimulationClock() : currentTime(0.0) {}

    void advanceToNextEvent() {
        if (!eventQueue.isEmpty()) {
            eventQueue.processNextEvent(currentTime);
        }
    }

    double getCurrentTime() {
        return currentTime;
    }

    void scheduleEvent(double time, function<void()> action) {
        eventQueue.addEvent(time, action);
    }

    bool isEmpty() {
        return eventQueue.isEmpty();
    }
};
//User Input
class UserInput {
private:
    static UserInput* instance;
    int machineCount;
    int shiftTiming;

    UserInput() : machineCount(6), shiftTiming(8) {}

public:
    static UserInput* getInstance() {
        if (!instance) {
            instance = new UserInput();
        }
        return instance;
    }

    int getMachineCount() const {
        return machineCount;
    }

    void setMachineCount(int count) {
        machineCount = count;
    }

    int getShiftTiming() const {
        return shiftTiming;
    }

    void setShiftTiming(int timing) {
        shiftTiming = timing;
    }
};

UserInput* UserInput::instance = nullptr;

// Simulation Data
class SimulationData {
public:
    static double getOperationTime(const string& operation) {
        if (operation == "RawMaterialHandling") return 2.0;
        if (operation == "Machining") return 5.0;
        if (operation == "Assembling") return 3.0;
        if (operation == "Inspecting") return 1.0;
        if (operation == "Packaging") return 2.0;
        return 0.0;
    }

    static double getAdjustedOperationTime(const string& operation) {
    int machineCount = UserInput::getInstance()->getMachineCount();
    int shiftTiming = UserInput::getInstance()->getShiftTiming();

    double baseOperationTime = getOperationTime(operation);

    // Adjust based on machine count and shift timing
    double adjustedOperationTime = baseOperationTime / static_cast<double>(machineCount * shiftTiming);

    return adjustedOperationTime;
}


    static double getFailureRate(const string& machine) {
        if (machine == "Machine1") return 0.05;
        if (machine == "Machine2") return 0.1;
        return 0.0;
    }

    static int getShiftLength() {
        return 8; // Default shift length
    }

    static int getWorkersPerShift() {
        return 5; // Default workers per shift
    }
};


// System Modeling: Production Stages
class ProductionStage {
protected:
    string name;
    vector<Machine*> machines;
    vector<Operator*> operators;
    double startTime;
    double endTime;
    SimulationClock& simulationClock;

public:
    ProductionStage(const string& name, SimulationClock& clock) : name(name), startTime(0.0), endTime(0.0), simulationClock(clock) {}

    string getName() const {
        return name;
    }

    virtual void process(Material* material) = 0;

    void addMachine(Machine* machine) {
        machines.push_back(machine);
    }

    void addOperator(Operator* oper) {
        operators.push_back(oper);
    }

    double getStartTime() const {
        return startTime;
    }

    double getEndTime() const {
        return endTime;
    }

    void setStartTime(double time) {
        startTime = time;
    }

    void setEndTime(double time) {
        endTime = time;
    }

    virtual double getOperationTime() const = 0;

    bool checkMachineFailure(Machine* machine) {
        double failureRate = SimulationData::getFailureRate(machine->name);
        return (static_cast<double>(rand()) / RAND_MAX) < failureRate;
    }
};

class RawMaterialHandling : public ProductionStage {
public:
    RawMaterialHandling(const string& name, SimulationClock& clock) : ProductionStage(name, clock) {}

    void process(Material* material) override {
        startTime = simulationClock.getCurrentTime();
        cout << "Processing raw material: " << material->name << " in " << name << endl;

        // Check machine failures
        for (auto& machine : machines) {
            if (checkMachineFailure(machine)) {
                cout << "Machine " << machine->name << " failed during " << name << endl;
                // Schedule a repair event
                simulationClock.scheduleEvent(simulationClock.getCurrentTime() + 1.0, [machine]() {
                    machine->release();
                    cout << "Machine " << machine->name << " repaired." << endl;
                });
                machine->use();
                return;
            }
        }

        // Adjust operation time based on user input
        double operationTime = SimulationData::getAdjustedOperationTime("RawMaterialHandling");
        this_thread::sleep_for(chrono::seconds(static_cast<int>(operationTime)));
        endTime = simulationClock.getCurrentTime();
    }

    double getOperationTime() const override {
        return SimulationData::getOperationTime("RawMaterialHandling");
    }
};


class Machining : public ProductionStage {
public:
    Machining(const string& name, SimulationClock& clock) : ProductionStage(name, clock) {}

    void process(Material* material) override {
        startTime = simulationClock.getCurrentTime();
        cout << "Machining material: " << material->name << " in " << name << endl;

        // Check machine failures
        for (auto& machine : machines) {
            if (checkMachineFailure(machine)) {
                cout << "Machine " << machine->name << " failed during " << name << endl;
                // Schedule a repair event
                simulationClock.scheduleEvent(simulationClock.getCurrentTime() + 1.0, [machine]() {
                    machine->release();
                    cout << "Machine " << machine->name << " repaired." << endl;
                });
                machine->use();
                return;
            }
        }

        // Adjust operation time based on user input
        double operationTime = SimulationData::getAdjustedOperationTime("Machining");
        this_thread::sleep_for(chrono::seconds(static_cast<int>(operationTime)));
        endTime = simulationClock.getCurrentTime();
    }

    double getOperationTime() const override {
        return SimulationData::getOperationTime("Machining");
    }
};

class Assembling : public ProductionStage {
public:
    Assembling(const string& name, SimulationClock& clock) : ProductionStage(name, clock) {}

    void process(Material* material) override {
        startTime = simulationClock.getCurrentTime();
        cout << "Assembling material: " << material->name << " in " << name << endl;

        // Check machine failures
        for (auto& machine : machines) {
            if (checkMachineFailure(machine)) {
                cout << "Machine " << machine->name << " failed during " << name << endl;
                // Schedule a repair event
                simulationClock.scheduleEvent(simulationClock.getCurrentTime() + 1.0, [machine]() {
                    machine->release();
                    cout << "Machine " << machine->name << " repaired." << endl;
                });
                machine->use();
                return;
            }
        }

        // Adjust operation time based on user input
        double operationTime = SimulationData::getAdjustedOperationTime("Assembling");
        this_thread::sleep_for(chrono::seconds(static_cast<int>(operationTime)));
        endTime = simulationClock.getCurrentTime();
    }

    double getOperationTime() const override {
        return SimulationData::getOperationTime("Assembling");
    }
};


class Inspecting : public ProductionStage {
public:
    Inspecting(const string& name, SimulationClock& clock) : ProductionStage(name, clock) {}

    void process(Material* material) override {
        startTime = simulationClock.getCurrentTime();
        cout << "Inspecting material: " << material->name << " in " << name << endl;

        // Check machine failures
        for (auto& machine : machines) {
            if (checkMachineFailure(machine)) {
                cout << "Machine " << machine->name << " failed during " << name << endl;
                // Schedule a repair event
                simulationClock.scheduleEvent(simulationClock.getCurrentTime() + 1.0, [machine]() {
                    machine->release();
                    cout << "Machine " << machine->name << " repaired." << endl;
                });
                machine->use();
                return;
            }
        }

        // Adjust operation time based on user input
        double operationTime = SimulationData::getAdjustedOperationTime("Inspecting");
        this_thread::sleep_for(chrono::seconds(static_cast<int>(operationTime)));
        endTime = simulationClock.getCurrentTime();
    }

    double getOperationTime() const override {
        return SimulationData::getOperationTime("Inspecting");
    }
};


class Packaging : public ProductionStage {
public:
    Packaging(const string& name, SimulationClock& clock) : ProductionStage(name, clock) {}

    void process(Material* material) override {
        startTime = simulationClock.getCurrentTime();
        cout << "Packaging material: " << material->name << " in " << name << endl;

        // Check machine failures
        for (auto& machine : machines) {
            if (checkMachineFailure(machine)) {
                cout << "Machine " << machine->name << " failed during " << name << endl;
                // Schedule a repair event
                simulationClock.scheduleEvent(simulationClock.getCurrentTime() + 1.0, [machine]() {
                    machine->release();
                    cout << "Machine " << machine->name << " repaired." << endl;
                });
                machine->use();
                return;
            }
        }

        // Adjust operation time based on user input
        double operationTime = SimulationData::getAdjustedOperationTime("Packaging");
        this_thread::sleep_for(chrono::seconds(static_cast<int>(operationTime)));
        endTime = simulationClock.getCurrentTime();
    }

    double getOperationTime() const override {
        return SimulationData::getOperationTime("Packaging");
    }
};


// Product Type
class ProductType {
public:
    string name;
    double setupTime;
    vector<string> stages;

    ProductType(const string& name, double setupTime, const vector<string>& stages)
        : name(name), setupTime(setupTime), stages(stages) {}
};

// Multi-Product Manufacturing System
class MultiProductManufacturingSystem {
private:
    vector<ProductionStage*> stages;
    vector<ProductType> productTypes;
    SimulationClock simulationClock;

public:
    MultiProductManufacturingSystem() {
        stages.push_back(new RawMaterialHandling("Raw Material Handling", simulationClock));
        stages.push_back(new Machining("Machining", simulationClock));
        stages.push_back(new Assembling("Assembling", simulationClock));
        stages.push_back(new Inspecting("Inspecting", simulationClock));
        stages.push_back(new Packaging("Packaging", simulationClock));

        // Create machines and operators
        Machine* machine1 = new Machine("Machine1");
        Machine* machine2 = new Machine("Machine2");
        Operator* operator1 = new Operator("Operator1");
        Operator* operator2 = new Operator("Operator2");

        // Add machines and operators to appropriate stages
        for (auto stage : stages) {
            stage->addMachine(machine1);
            stage->addMachine(machine2);

            stage->addOperator(operator1);
            stage->addOperator(operator2);
        }
    }

    void addProductType(const ProductType& productType) {
        productTypes.push_back(productType);
    }

    void runSingleSimulation() {
        Material* material = new Material("Single Material 1");
        for (auto stage : stages) {
            stage->process(material);
        }
    }

    void runMultiProductSimulation() {
        for (const auto& productType : productTypes) {
            cout << "Processing product type: " << productType.name << endl;
            this_thread::sleep_for(chrono::seconds((int)productType.setupTime));

            for (const auto& stageName : productType.stages) {
                for (auto& stage : stages) {
                    if (stage->getName() == stageName) {
                        Material* material = new Material(productType.name);
                        stage->setStartTime(simulationClock.getCurrentTime());
                        stage->process(material);
                        stage->setEndTime(simulationClock.getCurrentTime());
                    }
                }
            }

            cout << "---------------------------------------------" << endl;
        }
    }

    void analyzeBottlenecks() {
        for (auto stage : stages) {
            cout << "Bottlenecks Stage: " << stage->getName() << endl;
        }
    }

    void showOperationTimes() {
    double totalProcessingTime = 0.0;

    for (auto stage : stages) {
        double operationTime = SimulationData::getAdjustedOperationTime(stage->getName());
        cout << stage->getName() << " Time: " << operationTime << endl;
        totalProcessingTime += operationTime;
    }

    double totalIdleTime = simulationClock.getCurrentTime() - totalProcessingTime;

    cout << "Total Processing Time: " << totalProcessingTime << endl;
    cout << "Total Idle Time: " << totalIdleTime << endl;
}

};

// Experimentation and Analysis
class Experimentation {
public:
    void runSingleScenarios(MultiProductManufacturingSystem& system) {
        system.runSingleSimulation();
        system.analyzeBottlenecks();
        system.showOperationTimes();
    }

    void runMultiScenarios(MultiProductManufacturingSystem& system) {
        system.runMultiProductSimulation();
        system.analyzeBottlenecks();
        system.showOperationTimes();
    }

    void adjustVariablesAndObserve(MultiProductManufacturingSystem& system) {
        runMultiScenarios(system);
    }
};

// Main function
int main() {

    cout << "---------------------------------------------" << endl;
    cout << "Single Product" << endl;
    cout << "---------------------------------------------" << endl;

    MultiProductManufacturingSystem system;

    Experimentation experimentation;
    experimentation.runSingleScenarios(system);

    cout << "---------------------------------------------" << endl;
    cout << "Multi Product (Mercedes)" << endl;
    cout << "---------------------------------------------" << endl;

    system.addProductType(ProductType("C 180", 3.0, { "Raw Material Handling", "Machining", "Assembling", "Inspecting", "Packaging" }));
    system.addProductType(ProductType("E 200", 5.0, { "Raw Material Handling", "Machining", "Inspecting", "Packaging" }));
    system.addProductType(ProductType("S 600", 4.0, { "Raw Material Handling", "Machining", "Packaging" }));

    experimentation.runMultiScenarios(system);

    cout << "---------------------------------------------" << endl;
    cout << "Adjusting variables" << endl;
    cout << "---------------------------------------------" << endl;

    int machineCount;
    cout << "Input Machine Count: ";
    cin >> machineCount; 

    cout << endl;

    int shiftTiming;
    cout << "Input Shift Timing: ";
    cin >> shiftTiming;

    UserInput::getInstance()->setMachineCount(machineCount);
    UserInput::getInstance()->setShiftTiming(shiftTiming);
    experimentation.adjustVariablesAndObserve(system);


    return 0;
}


