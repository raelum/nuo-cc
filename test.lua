--[[
  Lua table based class.
]]
-- local Person = {}

-- function Person.new(name, money)
--   local self = setmetatable({}, { __index = Person })
--   self.name = name
--   self.money = money
--   return self
-- end

-- function Person:pay(amount)
--   self.money = self.money + amount
-- end

-- local alice = Person.new("alice", 5)
-- local bob = Person.new("bob", 10)

-- alice:pay(10)
-- print(alice.money)

-- bob:pay(12)
-- print(bob.money)

--[[
  Structs version. Issue here is cdata references to other cdata is not traced
  by the GC and they need to be held by a lua value somewhere for it to work.

  I tried to think of reference counting but it just doesn't work in Lua with
  no built-in copy-constructor or delete-constructor.

]]
local ffi = require("ffi")

ffi.cdef [[
typedef struct {
  const char* name;
  int money;
} Person;
]]

local Person = {}
local PersonType = ffi.metatype(ffi.typeof("Person"), { __index = Person })

function Person.new(name, money)
  return PersonType(name, money)
end

function Person:pay(amount)
  self.money = self.money + amount
end

local alice = Person.new("Alice", 5)
local bob = Person.new("Bob", 10)

alice:pay(6)
bob:pay(7)

print(ffi.string(alice.name))
print(alice.money)

print(ffi.string(bob.name))
print(bob.money)

alice.cool = 5
