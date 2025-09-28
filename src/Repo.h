#pragma once
#include <memory>
#include <optional>
#include <unordered_map>

template<typename Key, typename Resource>
class RepoBase
{
protected:
  std::unordered_map<Key, std::shared_ptr<Resource>> m_resources;

  template<typename... Args>
  std::shared_ptr<Resource> create(Key key, Args&&... args)
  {
    auto resource = std::make_shared<Resource>(std::forward<Args>(args)...);
    m_resources[key] = resource;
    return resource;
  }

public:
  virtual ~RepoBase() = default;

  std::optional<std::shared_ptr<Resource>> get(const Key& id)
  {
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  void remove(const Key& id) { m_resources.erase(id); }
  void clear() { m_resources.clear(); }
};
