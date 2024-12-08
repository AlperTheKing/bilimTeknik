count = 0

# Tüm kırmızı dağılımları (R1,R2,R3) için
for R1 in range(6):  # R1:0..5
    for R2 in range(6 - R1):  # R2:0..(5-R1)
        R3 = 5 - (R1 + R2)
        
        # Mavi toplar (B1,B2,B3)
        for B1 in range(5):  # B1:0..4
            for B2 in range(5 - B1): # B2:0..(4-B1)
                B3 = 4 - (B1 + B2)
                
                # Beyaz toplar (W1,W2,W3)
                for W1 in range(4): # W1:0..3
                    for W2 in range(4 - W1): # W2:0..(3-W1)
                        W3 = 3 - (W1 + W2)
                        
                        # Şimdi her kutunun renk dağılımına bakalım:
                        # 1. kutu: R1,B1,W1
                        # 2. kutu: R2,B2,W2
                        # 3. kutu: R3,B3,W3
                        
                        # Her kutuda en az iki farklı renk olacak mı?
                        # Bunu kontrol etmek için, her kutudaki
                        # sıfırdan büyük renk sayısına bakalım.
                        
                        box1_colors = sum([1 for x in [R1,B1,W1] if x > 0])
                        box2_colors = sum([1 for x in [R2,B2,W2] if x > 0])
                        box3_colors = sum([1 for x in [R3,B3,W3] if x > 0])
                        
                        # Koşul: her kutudaki farklı renk sayısı >= 2
                        if box1_colors >= 2 and box2_colors >= 2 and box3_colors >= 2:
                            count += 1

print(count)