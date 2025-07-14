#pragma once

UENUM(BlueprintType)
enum class EResourceType : uint8	
{
	Wood UMETA(DisplayName = "Wood"),
	Food UMETA(DisplayName = "Food"),
	Stone UMETA(DisplayName = "Stone"),
	Gold UMETA(DisplayName = "Gold"),
	Population UMETA(DisplayName = "Population")
};

UENUM(BlueprintType)
enum class EActorType : uint8	
{	
	//Pawn Type
	Villager UMETA(DisplayName = "Villager"),
	Swordman UMETA(DisplayName = "Swordman"),
	Knight UMETA(DisplayName = "Knight"),
	Archer UMETA(DisplayName = "Archer"),
	
	//Building Type
	House UMETA(DisplayName = "House"),
	Wall UMETA(DisplayName = "Wall"),
	Tree UMETA(DisplayName = "Tree")
};
