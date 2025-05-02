local Person = {}




function Person.new(name, money)
   local self = setmetatable({}, { __index = Person })
   self.name = name
   self.money = money
   return self
end

function Person:pay(amount)
   self.money = self.money + amount
end

local alice = Person.new("alice", 5)
local bob = Person.new("bob", 10)

alice:pay(10)
print(alice.money)

bob:pay(12)
print(bob.money)
