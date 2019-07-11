#include <iostream>
#include <mutex>

// http://www.informit.com/articles/article.aspx?p=25298&seqNum=7

template<class T>
class StrictLock
{
    T &obj_;

public:
    StrictLock() = delete; // disable default constructor
    StrictLock(const StrictLock &) = delete; // disable copy constructor
    StrictLock &operator=(const StrictLock &) = delete; // disable assignment
    void *operator new(std::size_t) = delete; // disable heap allocation

    void *operator new[](std::size_t) = delete;

    void operator delete(void *) = delete;

    void operator delete[](void *) = delete;
    StrictLock *operator&() = delete; // disable address taking
    explicit StrictLock(T &obj) :
            obj_(obj)
    {
        obj.AcquireMutex();
    }

    ~StrictLock()
    {
        obj_.ReleaseMutex();
    }
};

class BankAccount
{
    int balance_;
public:
    BankAccount() :
            balance_(0)
    {
    }

    void Deposit(int amount)
    {
        balance_ += amount;
    }

    void Withdraw(int amount)
    {
        balance_ -= amount;
    }
};

template<class T, class Owner>
class ExternallyLocked
{
    T obj_;
public:
    ExternallyLocked() = default;

    explicit ExternallyLocked(const T &obj) :
            obj_(obj)
    {
    }

    T &Get(StrictLock<Owner> &)
    {
        return obj_;
    }

    void Set(const T &obj, StrictLock<Owner> &)
    {
        obj_ = obj;
    }
};

class AccountManager
{
    std::mutex mtx_;
    ExternallyLocked<BankAccount, AccountManager> checkingAcct_;
    ExternallyLocked<BankAccount, AccountManager> savingsAcct_;
public:

    void AcquireMutex()
    {
        mtx_.lock();
    }

    void ReleaseMutex()
    {
        mtx_.unlock();
    }

    void Checking2Savings(int amount)
    {
        StrictLock<AccountManager> guard(*this);
        checkingAcct_.Get(guard).Withdraw(amount);
        savingsAcct_.Get(guard).Deposit(amount);
    }
};

int main()
{
    AccountManager().Checking2Savings(2);
    return 0;
}