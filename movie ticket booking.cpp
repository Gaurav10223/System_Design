#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
using namespace std;

// ---------- Seat State ----------
enum class SeatState { AVAILABLE, LOCKED, BOOKED };

// ---------- Seat ----------
class Seat {
    int number;
    SeatState state;
    string lockedBy;
    mutex mtx;

public:
    Seat(int n) : number(n), state(SeatState::AVAILABLE) {}

    bool lock(string user) {
        lock_guard<mutex> lock(mtx);
        if (state == SeatState::AVAILABLE) {
            state = SeatState::LOCKED;
            lockedBy = user;
            return true;
        }
        return false;
    }

    bool confirm(string user) {
        lock_guard<mutex> lock(mtx);
        if (state == SeatState::LOCKED && lockedBy == user) {
            state = SeatState::BOOKED;
            return true;
        }
        return false;
    }

    void unlock(string user) {
        lock_guard<mutex> lock(mtx);
        if (state == SeatState::LOCKED && lockedBy == user)
            state = SeatState::AVAILABLE;
    }
};

// ---------- Show ----------
class Show {
    int id;
    double price;
    vector<Seat> seats;

public:
    Show(int id, int totalSeats, double price)
        : id(id), price(price) {
        for (int i = 1; i <= totalSeats; i++)
            seats.emplace_back(i);
    }

    vector<Seat>& getSeats() { return seats; }
    double getPrice() { return price; }
};

// ---------- Payment Strategy ----------
class PaymentStrategy {
public:
    virtual bool pay(double amount) = 0;
    virtual ~PaymentStrategy() {}
};

class UpiPayment : public PaymentStrategy {
public:
    bool pay(double amount) override {
        cout << "Paid " << amount << " via UPI\n";
        return true;
    }
};

// ---------- Booking Manager (Singleton) ----------
class BookingManager {
    map<int, Show> shows;
    mutex mtx;
    static BookingManager* instance;

    BookingManager() {}

public:
    static BookingManager* getInstance() {
        if (!instance) instance = new BookingManager();
        return instance;
    }

    void addShow(int id, Show show) {
        lock_guard<mutex> lock(mtx);
        shows[id] = show;
    }

    bool book(int showId, vector<int> nums,
              string user, PaymentStrategy* payment) {

        Show& show = shows[showId];
        auto& seats = show.getSeats();

        // 1. Lock seats
        for (int n : nums) {
            if (!seats[n-1].lock(user)) {
                for (int x : nums) seats[x-1].unlock(user);
                cout << "Seats unavailable for " << user << "\n";
                return false;
            }
        }

        double amount = nums.size() * show.getPrice();

        // 2. Payment
        if (!payment->pay(amount)) {
            for (int n : nums) seats[n-1].unlock(user);
            return false;
        }

        // 3. Confirm
        for (int n : nums) seats[n-1].confirm(user);

        cout << "Booking success for " << user << "\n";
        return true;
    }
};

BookingManager* BookingManager::instance = nullptr;

int main() {
    BookingManager* mgr = BookingManager::getInstance();
    mgr->addShow(1, Show(1, 5, 200));

    UpiPayment upi;

    thread t1([&]() { mgr->book(1, {1,2}, "UserA", &upi); });
    thread t2([&]() { mgr->book(1, {2,3}, "UserB", &upi); });

    t1.join();
    t2.join();

    return 0;
}
