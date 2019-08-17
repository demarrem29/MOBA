// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "MOBACharacter.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	BottomSide		UMETA(DisplayName = "Bottom Side"),
	TopSide			UMETA(DisplayName = "Top Side"),
	NeutralHostile	UMETA(DisplayName = "Jungle Camps"),
	NeutralFriendly UMETA(DisplayName = "Shop Vendors"),
};

UENUM(BlueprintType)
enum class AbilityInput : uint8
{
	UseAbility1 UMETA(DisplayName = "Use Spell 1"), //This maps the first ability(input ID should be 0 in int) to the action mapping(which you define in the project settings) by the name of "UseAbility1". "Use Spell 1" is the blueprint name of the element.
	UseAbility2 UMETA(DisplayName = "Use Spell 2"), //Maps ability 2(input ID 1) to action mapping UseAbility2. "Use Spell 2" is mostly used for when the enum is a blueprint variable.
	UseAbility3 UMETA(DisplayName = "Use Spell 3"),
	UseAbility4 UMETA(DisplayName = "Use Spell 4"),
	WeaponAbility UMETA(DisplayName = "Use Weapon"), //This finally maps the fifth ability(here designated to be your weaponability, or auto-attack, or whatever) to action mapping "WeaponAbility".

		//You may also do something like define an enum element name that is not actually mapped to an input, for example if you have a passive ability that isn't supposed to have an input. This isn't usually necessary though as you usually grant abilities via input ID,
		//which can be negative while enums cannot. In fact, a constant called "INDEX_NONE" exists for the exact purpose of rendering an input as unavailable, and it's simply defined as -1.
		//Because abilities are granted by input ID, which is an int, you may use enum elements to describe the ID anyway however, because enums are fancily dressed up ints.
};

UCLASS(Blueprintable)
class AMOBACharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMOBACharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ability System")
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ability System")
		class UMOBAAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
		ETeam MyTeam;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;


	void AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
		void HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth);
	UFUNCTION()
		void ManaChange(FGameplayAttributeData mana, FGameplayAttributeData maxmana);
	UFUNCTION()
		void LevelChange(FGameplayAttributeData level, FGameplayAttributeData maxlevel);
	UFUNCTION()
		void ExperienceChange(FGameplayAttributeData experience, FGameplayAttributeData maxexperience);

	UFUNCTION(BlueprintImplementableEvent)
		void BP_HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ManaChange(FGameplayAttributeData mana, FGameplayAttributeData maxmana);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_LevelChange(FGameplayAttributeData level, FGameplayAttributeData maxlevel);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ExperienceChange(FGameplayAttributeData experience, FGameplayAttributeData maxexperience);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

};

