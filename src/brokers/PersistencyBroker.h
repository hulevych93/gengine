#pragma once

#include <string>
#include <boost/signals2/connection.hpp>

namespace Gengine {

namespace Serialization {
class ISerializable;
}

namespace JSON {
class IJsonSerializable;
}

namespace Services {

using connection = std::vector<boost::signals2::scoped_connection>;

class IPersistency
{
public:
    virtual ~IPersistency() = default;
    virtual void operator()(const Serialization::ISerializable& object) = 0;
    virtual void operator()(Serialization::ISerializable& object) const = 0;
    virtual void operator()(const JSON::IJsonSerializable& object) = 0;
    virtual void operator()(JSON::IJsonSerializable& object) const = 0;
};

class IPersistencyClient
{
public:
    virtual ~IPersistencyClient() = default;
    virtual void OnPersistency(const IPersistency& persistency) = 0;
    virtual void OnPersistency(IPersistency& persistency) const = 0;
};

class IPersistencyBroker
{
public:
    virtual ~IPersistencyBroker() = default;
    virtual void Configure(const std::string& directory) = 0;
    virtual void Deconfigure() = 0;
    virtual connection Subscribe(const std::string& id, IPersistencyClient& client) = 0;
    virtual void Load(const std::string& id) = 0;
    virtual void Store(const std::string& id) const = 0;
    virtual void Delete(const std::string& id) = 0;
};

class PersistableBase : public IPersistencyClient
{
public:
    explicit PersistableBase(const std::string& id);
    ~PersistableBase();

    void Load();
    void Store();
    void Delete();

private:
    const std::string m_id;
    connection m_endpoint;
};

template<class Data>
class PersistableData: public PersistableBase
{
protected:
    void OnPersistency(const IPersistency& persistency) override
    {
        persistency(m_data);
    }
    void OnPersistency(IPersistency& persistency) const override
    {
        persistency(m_data);
    }

private:
    Data m_data;
};

template<class Data>
class Persistable : public PersistableBase,
                    public Data
{
protected:
    explicit Persistable(const std::string& id)
    : PersistableBase(id)
    {}
    
    void OnPersistency(const IPersistency& persistency) override
    {
        if (m_initialized)
        {
            persistency(GetThis());
        }
    }
    void OnPersistency(IPersistency& persistency) const override
    {
        if (!m_initialized)
        {
            persistency(GetThis());
            m_initialized = true;
        }
    }

private:
    Data & GetThis()
    {
        return *static_cast<Data*>(this);
    }

    const Data & GetThis() const
    {
        return *static_cast<const Data*>(this);
    }

private:
    mutable bool m_initialized = false;
};

void InitializePersistency(const std::string& directory);
void DeinitializePersistency();

#define INITIALIZE_PERSISTENCY(config) InitializePersistency(config)
#define UNINITIALIZE_PERSISTENCY DeinitializePersistency()

}
}

