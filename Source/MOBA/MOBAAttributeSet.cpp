// Fill out your copyright notice in the Description page of Project Settings.


#include "MOBAAttributeSet.h"

UMOBAAttributeSet::UMOBAAttributeSet() 
	:Health(500.0f)
	,MaxHealth(500.0f)
	,HealthRegen(7.0f)
	,Mana(300.0f)
	,MaxMana(300.0f)
	,ManaRegen(7.3f)
	,Level(1.0f)
	,MaxLevel(18.0f)
	,Experience(0.0f)
	,MaxExperience(280.0f)
	,AttackPower(55.0f)
	,SpellPower(0.0f)
	,AttackSpeed(0.7f)
	,CriticalChance(0.1f)
	,CriticalDamage(2.0f)
	,AttackRange(150.0f)
	,Armor(0.0f)
	,PhysicalDamageReduction(0.0f)
	,EnvironmentalResistance(0.0f)
	,EnvironmentalDamageReduction(0.0f)
	,FlatDamageReduction(0.0f)
	,MovementSpeed(600.0f)
{
	// Construct XP per Level Data Table
	static ConstructorHelpers::FObjectFinder<UDataTable>
		ExperiencePerLevelObject(TEXT("DataTable'/Game/Data/ExperiencePerLevel.ExperiencePerLevel'"));
	if (ExperiencePerLevelObject.Succeeded())
	{
		ExperiencePerLevelData = ExperiencePerLevelObject.Object;
	}
}

float UMOBAAttributeSet::CalculateDamageReduction(float ResistanceStat) 
{
	float damagereduction;
	if (ResistanceStat >= 0)
	{
		damagereduction = 1 - (100 / (100 + ResistanceStat));
	}
	else 
	{
		damagereduction = 1 - (2 - (100 / (100 - ResistanceStat)));
	}
	return damagereduction;
}

void UMOBAAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* Source = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();

	if (HealthAttribute() == Data.EvaluatedData.Attribute)
	{
		// Get the Target actor
		AActor* DamagedActor = nullptr;
		AController* DamagedController = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			DamagedActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			DamagedController = Data.Target.AbilityActorInfo->PlayerController.Get();
		}

		// Get the Source actor
		AActor* AttackingActor = nullptr;
		AController* AttackingController = nullptr;
		AController* AttackingPlayerController = nullptr;
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			AttackingActor = Source->AbilityActorInfo->AvatarActor.Get();
			AttackingController = Source->AbilityActorInfo->PlayerController.Get();
			AttackingPlayerController = Source->AbilityActorInfo->PlayerController.Get();
			if (AttackingController == nullptr && AttackingActor != nullptr)
			{
				if (APawn * Pawn = Cast<APawn>(AttackingActor))
				{
					AttackingController = Pawn->GetController();
				}
			}
		}

		// Clamp health
		Health = FMath::Clamp(Health.GetCurrentValue(), 0.0f, MaxHealth.GetCurrentValue());
		if (Health.GetCurrentValue() <= 0)
		{
			/*
			// Handle death with GASCharacter. Note this is just one example of how this could be done.
			if (AGASCharacter * GASChar = Cast<AGASCharacter>(DamagedActor))
			{
				// Construct a gameplay cue event for this death
				FGameplayCueParameters Params(Data.EffectSpec.GetContext());
				Params.RawMagnitude = Data.EvaluatedData.Magnitude;;
				Params.NormalizedMagnitude = FMath::Abs(Data.EvaluatedData.Magnitude / MaxHealth);
				Params.AggregatedSourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
				Params.AggregatedTargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();

				GASChar->Die(DamagedController, DamagedActor, Data.EffectSpec, Params.RawMagnitude, Params.Normal);
			}*/
			HealthChange.Broadcast(Health,MaxHealth);
		}
	}
	if (HealthRegenAttribute() == Data.EvaluatedData.Attribute) 
	{
		HealthRegen = FMath::Clamp(HealthRegen.GetCurrentValue(), 0.0f, 9999.0f);
		HealthRegenChange.Broadcast(HealthRegen);
	}
	if (ManaAttribute() == Data.EvaluatedData.Attribute)
	{
		Mana = FMath::Clamp(Mana.GetCurrentValue(), 0.0f, MaxMana.GetCurrentValue());
		ManaChange.Broadcast(Mana, MaxMana);
	}
	if (ManaRegenAttribute() == Data.EvaluatedData.Attribute)
	{
		ManaRegen = FMath::Clamp(ManaRegen.GetCurrentValue(), 0.0f, 9999.0f);
		ManaRegenChange.Broadcast(ManaRegen);
	}
	if (LevelAttribute() == Data.EvaluatedData.Attribute) 
	{
		Level = FMath::Clamp(Level.GetCurrentValue(), 1.0f, MaxLevel.GetCurrentValue());
		if (Level.GetCurrentValue() == MaxLevel.GetCurrentValue()) 
		{
			Experience = 0;
			MaxExperience = 0;
		}
		LevelChange.Broadcast(Level, MaxLevel);
	}
	if (ExperienceAttribute() == Data.EvaluatedData.Attribute)
	{
		if (Level.GetCurrentValue() < MaxLevel.GetCurrentValue()) {		// Only check XP if level is less than max level
			FString currentlevelstring = FString::FromInt(static_cast<int32>(Level.GetCurrentValue()));	// Get current level as a string
			FExperiencePerLevel* XPToLvl = ExperiencePerLevelData->FindRow<FExperiencePerLevel>(FName(*currentlevelstring), "Experience Per Level", true); // Grab the row for the current level
			if (XPToLvl)
			{
				if (Experience.GetCurrentValue() >= XPToLvl->Experience) // Check if we have enough experience for a level up
				{
					Experience = 0;
					Level = (Level.GetCurrentValue() + 1);
					LevelChange.Broadcast(Level, MaxLevel);
					currentlevelstring = FString::FromInt(static_cast<int32>(Level.GetCurrentValue()));	// Get current level as a string
					XPToLvl = ExperiencePerLevelData->FindRow<FExperiencePerLevel>(FName(*currentlevelstring), "Experience Per Level", true); // Grab the row for the current level
					MaxExperience = XPToLvl->Experience;
				}
			}
		}
		else Experience = FMath::Clamp(Experience.GetCurrentValue(), 0.0f, 0.0f);
		ExperienceChange.Broadcast(Experience, MaxExperience);
	}
	if (AttackPowerAttribute() == Data.EvaluatedData.Attribute)
	{
		AttackPower = FMath::Clamp(AttackPower.GetCurrentValue(), 0.0f, 1000.0f);
		AttackPowerChange.Broadcast(AttackPower);
	}
	if (SpellPowerAttribute() == Data.EvaluatedData.Attribute)
	{
		SpellPower = FMath::Clamp(SpellPower.GetCurrentValue(), 0.0f, 2000.0f);
		SpellPowerChange.Broadcast(SpellPower);
	}
	if (AttackSpeedAttribute() == Data.EvaluatedData.Attribute)
	{
		AttackSpeed = FMath::Clamp(AttackSpeed.GetCurrentValue(), 0.0f, 2.5f);
		AttackSpeedChange.Broadcast(AttackSpeed);
	}
	if (CriticalChanceAttribute() == Data.EvaluatedData.Attribute)
	{
		CriticalChance = FMath::Clamp(CriticalChance.GetCurrentValue(), 0.0f, 1.0f);
		CriticalChanceChange.Broadcast(CriticalChance);
	}
	if (CriticalDamageAttribute() == Data.EvaluatedData.Attribute)
	{
		CriticalDamageChange.Broadcast(CriticalDamage);
	}
	if (AttackRangeAttribute() == Data.EvaluatedData.Attribute)
	{
		AttackRange = FMath::Clamp(AttackRange.GetCurrentValue(), 0.0f, 800.0f);
		AttackRangeChange.Broadcast(AttackRange);
	}
	if (ArmorAttribute() == Data.EvaluatedData.Attribute)
	{
		ArmorChange.Broadcast(Armor);
		PhysicalDamageReduction = CalculateDamageReduction(Armor.GetCurrentValue());
		PhysicalDamageReductionChange.Broadcast(PhysicalDamageReduction);
	}
	if (PhysicalDamageReductionAttribute() == Data.EvaluatedData.Attribute) 
	{
		PhysicalDamageReductionChange.Broadcast(PhysicalDamageReduction);
	}
	if (EnvironmentalResistanceAttribute() == Data.EvaluatedData.Attribute)
	{
		EnvironmentalResistanceChange.Broadcast(EnvironmentalResistance);
		EnvironmentalDamageReduction = CalculateDamageReduction(EnvironmentalResistance.GetCurrentValue());
		EnvironmentalDamageReductionChange.Broadcast(EnvironmentalDamageReduction);
	}
	if (EnvironmentalDamageReductionAttribute() == Data.EvaluatedData.Attribute)
	{
		EnvironmentalDamageReductionChange.Broadcast(EnvironmentalDamageReduction);
	}
	if (FlatDamageReductionAttribute() == Data.EvaluatedData.Attribute)
	{
		FlatDamageReductionChange.Broadcast(FlatDamageReduction);
	}
	if (MovementSpeedAttribute() == Data.EvaluatedData.Attribute)
	{
		MovementSpeedChange.Broadcast(MovementSpeed);
	}
}

FGameplayAttribute UMOBAAttributeSet::HealthAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Health));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::HealthRegenAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, HealthRegen));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ManaAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Mana));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ManaRegenAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, ManaRegen));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::LevelAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Level));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ExperienceAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Experience));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::AttackPowerAttribute() 
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, AttackPower));
	return FGameplayAttribute(Property);
}
FGameplayAttribute UMOBAAttributeSet::SpellPowerAttribute() 
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, SpellPower));
	return FGameplayAttribute(Property);
}
FGameplayAttribute UMOBAAttributeSet::AttackSpeedAttribute() 
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, AttackSpeed));
	return FGameplayAttribute(Property);
}
FGameplayAttribute UMOBAAttributeSet::CriticalChanceAttribute() 
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, CriticalChance));
	return FGameplayAttribute(Property);
}
FGameplayAttribute UMOBAAttributeSet::CriticalDamageAttribute() 
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, CriticalDamage));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::AttackRangeAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, AttackRange));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::ArmorAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, Armor));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::PhysicalDamageReductionAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, PhysicalDamageReduction));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::EnvironmentalResistanceAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, EnvironmentalResistance));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::EnvironmentalDamageReductionAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, EnvironmentalDamageReduction));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::FlatDamageReductionAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, FlatDamageReduction));
	return FGameplayAttribute(Property);
}

FGameplayAttribute UMOBAAttributeSet::MovementSpeedAttribute()
{
	static UProperty* Property = FindFieldChecked<UProperty>(UMOBAAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UMOBAAttributeSet, MovementSpeed));
	return FGameplayAttribute(Property);
}