#!/usr/bin/env python
# coding: utf-8

# 

# ### Parsing the full log data to structure it with file names as Y-axis and categories as X-axis
# 

# In[1]:


import pandas as pd


# In[2]:


# full_log_entries = """
# log4111:insertion_time_ns = 29840877160
# log4111:rd_time_ns = 6205742
# log4111:total_cpu_time_ns = 8997000.00
# log4111:whole block total_cpu_time_ns = 12997000.00
# log4111:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 13941803641
# log4111:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 9773206446
# log4111:all_time_ns = 459906510673
# log4112:insertion_time_ns = 29143038161
# log4112:rd_time_ns = 5551035
# log4112:total_cpu_time_ns = 7000000.00
# log4112:whole block total_cpu_time_ns = 12998000.00
# log4112:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 13277790510
# log4112:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 9400881872
# log4112:all_time_ns = 450914711024
# log4113:insertion_time_ns = 30757358723
# log4113:rd_time_ns = 6219290
# log4113:total_cpu_time_ns = 6000000.00
# log4113:whole block total_cpu_time_ns = 13000000.00
# log4113:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 13168233275
# log4113:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 10093249224
# log4113:all_time_ns = 438255866815
# log4211:insertion_time_ns = 27524521980
# log4211:rd_time_ns = 6107539
# log4211:total_cpu_time_ns = 6999000.00
# log4211:whole block total_cpu_time_ns = 13000000.00
# log4211:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10955484703
# log4211:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7237165317
# log4211:all_time_ns = 460922774571
# log4212:insertion_time_ns = 27638993173
# log4212:rd_time_ns = 6037403
# log4212:total_cpu_time_ns = 6999000.00
# log4212:whole block total_cpu_time_ns = 12999000.00
# log4212:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10360492961
# log4212:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7061245905
# log4212:all_time_ns = 452176163063
# log4213:insertion_time_ns = 27301415155
# log4213:rd_time_ns = 5917281
# log4213:total_cpu_time_ns = 7001000.00
# log4213:whole block total_cpu_time_ns = 12999000.00
# log4213:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10043559898
# log4213:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7591319823
# log4213:all_time_ns = 439934640764
# log4311:insertion_time_ns = 28022243337
# log4311:rd_time_ns = 5617931
# log4311:total_cpu_time_ns = 7998000.00
# log4311:whole block total_cpu_time_ns = 12998000.00
# log4311:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10999412382
# log4311:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4179697650
# log4311:all_time_ns = 449585418218
# log4312:insertion_time_ns = 28409597635
# log4312:rd_time_ns = 5785293
# log4312:total_cpu_time_ns = 5000000.00
# log4312:whole block total_cpu_time_ns = 12997000.00
# log4312:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10219359558
# log4312:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 3977548789
# log4312:all_time_ns = 437470714033
# log4313:insertion_time_ns = 29176951048
# log4313:rd_time_ns = 6099795
# log4313:total_cpu_time_ns = 2998000.00
# log4313:whole block total_cpu_time_ns = 12999000.00
# log4313:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 9988232663
# log4313:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4507992628
# log4313:all_time_ns = 426165098164
# log4411:insertion_time_ns = 28209554878
# log4411:rd_time_ns = 6224714
# log4411:total_cpu_time_ns = 6000000.00
# log4411:whole block total_cpu_time_ns = 12999000.00
# log4411:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11122211865
# log4411:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 34786825
# log4411:all_time_ns = 443211189513
# log4412:insertion_time_ns = 28505192471
# log4412:rd_time_ns = 5735601
# log4412:total_cpu_time_ns = 6000000.00
# log4412:whole block total_cpu_time_ns = 12999000.00
# log4412:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10289545328
# log4412:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 36173101
# log4412:all_time_ns = 431691656837
# log4413:insertion_time_ns = 29597342333
# log4413:rd_time_ns = 5906251
# log4413:total_cpu_time_ns = 8999000.00
# log4413:whole block total_cpu_time_ns = 12998000.00
# log4413:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10003352530
# log4413:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 34406870
# log4413:all_time_ns = 416881555782
# log4511:insertion_time_ns = 28864576902
# log4511:rd_time_ns = 5640032
# log4511:total_cpu_time_ns = 10000000.00
# log4511:whole block total_cpu_time_ns = 13000000.00
# log4511:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11016708283
# log4511:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 10174205823
# log4511:all_time_ns = 457729648786
# log4512:insertion_time_ns = 28606233708
# log4512:rd_time_ns = 5906975
# log4512:total_cpu_time_ns = 6999000.00
# log4512:whole block total_cpu_time_ns = 12999000.00
# log4512:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10248340689
# log4512:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 9751197370
# log4512:all_time_ns = 451058512698
# log4513:insertion_time_ns = 25960485129
# log4513:rd_time_ns = 6162059
# log4513:total_cpu_time_ns = 6000000.00
# log4513:whole block total_cpu_time_ns = 12999000.00
# log4513:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10054521961
# log4513:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 10172391022
# log4513:all_time_ns = 457796027416
# log4611:insertion_time_ns = 26924108589
# log4611:rd_time_ns = 5518172
# log4611:total_cpu_time_ns = 10001000.00
# log4611:whole block total_cpu_time_ns = 12999000.00
# log4611:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11272788411
# log4611:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7311900420
# log4611:all_time_ns = 453709429769
# log4612:insertion_time_ns = 30080542867
# log4612:rd_time_ns = 6267018
# log4612:total_cpu_time_ns = 6001000.00
# log4612:whole block total_cpu_time_ns = 12999000.00
# log4612:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10511401620
# log4612:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7170020399
# log4612:all_time_ns = 441800746590
# log4613:insertion_time_ns = 29488979521
# log4613:rd_time_ns = 6341950
# log4613:total_cpu_time_ns = 4001000.00
# log4613:whole block total_cpu_time_ns = 12999000.00
# log4613:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10277423932
# log4613:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7641742806
# log4613:all_time_ns = 430769685104
# log4711:insertion_time_ns = 29061039967
# log4711:rd_time_ns = 6028726
# log4711:total_cpu_time_ns = 4000000.00
# log4711:whole block total_cpu_time_ns = 12999000.00
# log4711:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11247246000
# log4711:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4673045953
# log4711:all_time_ns = 447987769798
# log4712:insertion_time_ns = 28838005884
# log4712:rd_time_ns = 5677590
# log4712:total_cpu_time_ns = 5999000.00
# log4712:whole block total_cpu_time_ns = 12999000.00
# log4712:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10520108817
# log4712:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4490506772
# log4712:all_time_ns = 436716686014
# log4713:insertion_time_ns = 28195966348
# log4713:rd_time_ns = 6260409
# log4713:total_cpu_time_ns = 5999000.00
# log4713:whole block total_cpu_time_ns = 13000000.00
# log4713:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10141877160
# log4713:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5356082080
# log4713:all_time_ns = 426999685364
# log4811:insertion_time_ns = 29023646684
# log4811:rd_time_ns = 6009031
# log4811:total_cpu_time_ns = 3001000.00
# log4811:whole block total_cpu_time_ns = 12999000.00
# log4811:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11440507932
# log4811:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7456048604
# log4811:all_time_ns = 451207636941
# log4812:insertion_time_ns = 29358303259
# log4812:rd_time_ns = 6098139
# log4812:total_cpu_time_ns = 7000000.00
# log4812:whole block total_cpu_time_ns = 13000000.00
# log4812:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10629764617
# log4812:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7217288791
# log4812:all_time_ns = 442301584691
# log4813:insertion_time_ns = 29575317320
# log4813:rd_time_ns = 5511680
# log4813:total_cpu_time_ns = 2000000.00
# log4813:whole block total_cpu_time_ns = 12999000.00
# log4813:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10297085406
# log4813:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7648972710
# log4813:all_time_ns = 429135319698
# log4911:insertion_time_ns = 29241230353
# log4911:rd_time_ns = 6339392
# log4911:total_cpu_time_ns = 8000000.00
# log4911:whole block total_cpu_time_ns = 13999000.00
# log4911:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11399303904
# log4911:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5548750963
# log4911:all_time_ns = 447776738523
# log4912:insertion_time_ns = 29092855486
# log4912:rd_time_ns = 5741202
# log4912:total_cpu_time_ns = 8000000.00
# log4912:whole block total_cpu_time_ns = 12000000.00
# log4912:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10548006210
# log4912:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5264992226
# log4912:all_time_ns = 438929649083
# log4913:insertion_time_ns = 30136534881
# log4913:rd_time_ns = 6073607
# log4913:total_cpu_time_ns = 5000000.00
# log4913:whole block total_cpu_time_ns = 12999000.00
# log4913:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10230266498
# log4913:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 6020954987
# log4913:all_time_ns = 424086024803
# log5011:insertion_time_ns = 29638452501
# log5011:rd_time_ns = 6190320
# log5011:total_cpu_time_ns = 4999000.00
# log5011:whole block total_cpu_time_ns = 12999000.00
# log5011:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11312426033
# log5011:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7348238392
# log5011:all_time_ns = 449565256043
# log5012:insertion_time_ns = 29397671345
# log5012:rd_time_ns = 6096328
# log5012:total_cpu_time_ns = 4999000.00
# log5012:whole block total_cpu_time_ns = 12999000.00
# log5012:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10527168965
# log5012:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7204097979
# log5012:all_time_ns = 442732920684
# log5013:insertion_time_ns = 28507250699
# log5013:rd_time_ns = 6276541
# log5013:total_cpu_time_ns = 5000000.00
# log5013:whole block total_cpu_time_ns = 11999000.00
# log5013:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10301157146
# log5013:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7689169783
# log5013:all_time_ns = 434068224965
# log5111:insertion_time_ns = 27463556462
# log5111:rd_time_ns = 6122094
# log5111:total_cpu_time_ns = 8000000.00
# log5111:whole block total_cpu_time_ns = 12999000.00
# log5111:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11345395439
# log5111:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5974516331
# log5111:all_time_ns = 454261709260
# log5112:insertion_time_ns = 29138131382
# log5112:rd_time_ns = 6221752
# log5112:total_cpu_time_ns = 3999000.00
# log5112:whole block total_cpu_time_ns = 12999000.00
# log5112:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10639915086
# log5112:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 6235868816
# log5112:all_time_ns = 440592017177
# log5113:insertion_time_ns = 29578268492
# log5113:rd_time_ns = 6058307
# log5113:total_cpu_time_ns = 7996000.00
# log5113:whole block total_cpu_time_ns = 12996000.00
# log5113:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10331944902
# log5113:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 6832908322
# log5113:all_time_ns = 428515232509
# log5211:insertion_time_ns = 27196936651
# log5211:rd_time_ns = 6102672
# log5211:total_cpu_time_ns = 7001000.00
# log5211:whole block total_cpu_time_ns = 12999000.00
# log5211:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11299494324
# log5211:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7338321383
# log5211:all_time_ns = 465837410577
# log5212:insertion_time_ns = 29359519519
# log5212:rd_time_ns = 5790506
# log5212:total_cpu_time_ns = 6999000.00
# log5212:whole block total_cpu_time_ns = 12999000.00
# log5212:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10551849513
# log5212:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7190111400
# log5212:all_time_ns = 440915309740
# log5213:insertion_time_ns = 29262042364
# log5213:rd_time_ns = 5609036
# log5213:total_cpu_time_ns = 5000000.00
# log5213:whole block total_cpu_time_ns = 13999000.00
# log5213:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10274886940
# log5213:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7712052242
# log5213:all_time_ns = 429908852963
# log5311:insertion_time_ns = 29528661620
# log5311:rd_time_ns = 5869043
# log5311:total_cpu_time_ns = 6000000.00
# log5311:whole block total_cpu_time_ns = 12999000.00
# log5311:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11476486842
# log5311:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4803938084
# log5311:all_time_ns = 445768184704
# log5312:insertion_time_ns = 29072979385
# log5312:rd_time_ns = 6115464
# log5312:total_cpu_time_ns = 5000000.00
# log5312:whole block total_cpu_time_ns = 13000000.00
# log5312:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10658606899
# log5312:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4552896919
# log5312:all_time_ns = 437624969047
# log5313:insertion_time_ns = 27755239503
# log5313:rd_time_ns = 5884167
# log5313:total_cpu_time_ns = 7000000.00
# log5313:whole block total_cpu_time_ns = 12999000.00
# log5313:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10515459640
# log5313:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5413215010
# log5313:all_time_ns = 438950908735
# log5411:insertion_time_ns = 29310439551
# log5411:rd_time_ns = 6077145
# log5411:total_cpu_time_ns = 5998000.00
# log5411:whole block total_cpu_time_ns = 12999000.00
# log5411:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11440520835
# log5411:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7483836334
# log5411:all_time_ns = 450733919364
# log5412:insertion_time_ns = 28828808155
# log5412:rd_time_ns = 5492421
# log5412:total_cpu_time_ns = 10999000.00
# log5412:whole block total_cpu_time_ns = 12999000.00
# log5412:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10453540985
# log5412:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7133775295
# log5412:all_time_ns = 442442856736
# log5413:insertion_time_ns = 30331912824
# log5413:rd_time_ns = 6244928
# log5413:total_cpu_time_ns = 6000000.00
# log5413:whole block total_cpu_time_ns = 12999000.00
# log5413:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10406487018
# log5413:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7760511618
# log5413:all_time_ns = 429992692079
# log5511:insertion_time_ns = 29448082116
# log5511:rd_time_ns = 6095172
# log5511:total_cpu_time_ns = 6999000.00
# log5511:whole block total_cpu_time_ns = 12999000.00
# log5511:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11511908250
# log5511:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4790424276
# log5511:all_time_ns = 447032394393
# log5512:insertion_time_ns = 27754631986
# log5512:rd_time_ns = 6197243
# log5512:total_cpu_time_ns = 4999000.00
# log5512:whole block total_cpu_time_ns = 13000000.00
# log5512:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10565027719
# log5512:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4492656374
# log5512:all_time_ns = 440752408759
# log5513:insertion_time_ns = 29666381878
# log5513:rd_time_ns = 6046428
# log5513:total_cpu_time_ns = 9997000.00
# log5513:whole block total_cpu_time_ns = 12998000.00
# log5513:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10258025003
# log5513:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5412148560
# log5513:all_time_ns = 425026176737
# log5611:insertion_time_ns = 28942056653
# log5611:rd_time_ns = 5711422
# log5611:total_cpu_time_ns = 9000000.00
# log5611:whole block total_cpu_time_ns = 12999000.00
# log5611:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11318224134
# log5611:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7458337878
# log5611:all_time_ns = 452358288554
# log5612:insertion_time_ns = 29209542807
# log5612:rd_time_ns = 6072382
# log5612:total_cpu_time_ns = 8001000.00
# log5612:whole block total_cpu_time_ns = 13999000.00
# log5612:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10731217401
# log5612:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7263120137
# log5612:all_time_ns = 442340119372
# log5613:insertion_time_ns = 29862944702
# log5613:rd_time_ns = 5774697
# log5613:total_cpu_time_ns = 4001000.00
# log5613:whole block total_cpu_time_ns = 12999000.00
# log5613:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10348931554
# log5613:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 7754527739
# log5613:all_time_ns = 429336991040
# log5711:insertion_time_ns = 29092430636
# log5711:rd_time_ns = 5758678
# log5711:total_cpu_time_ns = 6999000.00
# log5711:whole block total_cpu_time_ns = 11999000.00
# log5711:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 11447066063
# log5711:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4768241976
# log5711:all_time_ns = 445602610086
# log5712:insertion_time_ns = 28932060854
# log5712:rd_time_ns = 5475166
# log5712:total_cpu_time_ns = 1999000.00
# log5712:whole block total_cpu_time_ns = 12999000.00
# log5712:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10557915733
# log5712:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 4497009452
# log5712:all_time_ns = 436269011100
# log5713:insertion_time_ns = 30049255520
# log5713:rd_time_ns = 6198300
# log5713:total_cpu_time_ns = 4000000.00
# log5713:whole block total_cpu_time_ns = 12999000.00
# log5713:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 10234274092
# log5713:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 5420496208
# log5713:all_time_ns = 424729015086
# log5811:insertion_time_ns = 26877348019
# log5811:rd_time_ns = 6320767
# log5811:total_cpu_time_ns = 10999000.00
# log5811:whole block total_cpu_time_ns = 12999000.00
# log5811:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 22484976803
# log5811:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 14730445515
# log5811:all_time_ns = 576212894989
# log5812:insertion_time_ns = 24824478808
# log5812:rd_time_ns = 6177468
# log5812:total_cpu_time_ns = 4998000.00
# log5812:whole block total_cpu_time_ns = 12998000.00
# log5812:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 20501182566
# log5812:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 14130026822
# log5812:all_time_ns = 574488153052
# log5813:insertion_time_ns = 27092132702
# log5813:rd_time_ns = 5831242
# log5813:total_cpu_time_ns = 3988000.00
# log5813:whole block total_cpu_time_ns = 13000000.00
# log5813:fixed #PQ = 5000 point_query_time_on_existing_keys_ns = 20215506154
# log5813:fixed #PQ = 5000 point_query_time_on_currently_deleted_all_ns = 15126818789
# log5813:all_time_ns = 548269286149
# """


# In[3]:


with open("out_log.txt", "r") as f:
    full_log_entries = "".join(f.readlines())
# print(full_log_entries)


# In[ ]:





# In[4]:


strings_to_remove = [
    "NONE_CACHE_RANGETOMBSTONE_TRACING",
    "TOP_LEVEL_RDF_STRING_KEY",
    "SPLIT_PLRDF_STRING_KEY",
    "Split_PLRDF_STRING_KEY",
    "SuRF_LF_SPLIT_RDF",
    "PLRDF_STRING_KEY",
    "ROCKSDB (NONE)",
    "RocksDB (None)",
    "TOP_LEVEL_RDF",
    "SKYLINE_RDF",
    "SuRF_LF_RDF",
    "SPLIT_PLRDF",
    "Split_PLRDF",
    "NONE_DUMMY",
    "PLRDF",
    "NONE2",
    "NONE",
]


# In[5]:


# Creating a dictionary to store structured data
structured_data = {}

# Process each log entry
for line in full_log_entries.strip().split("\n"):
    parts = line.split(" = ")
    if len(parts) == 2:
        log_id, metric = parts[0].split(":", 1)
        # print(2, log_id, metric)
        value = float(parts[1]) if "." in parts[1] else int(parts[1], 0)

        for string in strings_to_remove:
            metric = metric.replace(string, "")
        
        if log_id not in structured_data:
            structured_data[log_id] = {}
        
        structured_data[log_id][metric] = value
    
    if len(parts) == 3:
        log_id, metric_1 = parts[0].split(":", 1)
        metric = metric_1 + " = " + parts[1]
        # print(3, log_id, metric)
        value = float(parts[2]) if "." in parts[2] else int(parts[2], 0)
        
        for string in strings_to_remove:
            metric = metric.replace(string, "")
        
        if log_id not in structured_data:
            structured_data[log_id] = {}
        
        structured_data[log_id][metric] = value
# print(structured_data['log2111'].keys())


# In[ ]:


# Define the required order based on the 4-digit prefix after "log"
# required_order = [11, 21, 31, 41, 51, 61, 71, 81, 91, 121, 131, 141, 151, 101, 111, 161, 171]
# offset = 4000

# required_order = [11, 21, 31, 41, 51, 61, 71, 81, 91, 101, 111, 121]
# required_order = [11, 21, 31, 41, 51, 61, 71, 81, 91]
required_order = [11, 21, 31, 41, 51, 61, 71, 81, 91, 101]
offset = 6000
n_group = 1

# print(str(list(structured_data.keys())))
filenames = list(structured_data.keys())

# Reinitialize a dictionary for ordered storage based on the correct mapping
sorted_filenames_corrected = {key: [] for key in required_order}

# Process each filename and categorize based on the given order
for filename in filenames:
    # num_part = (int(filename[3:7]) % 10)
    num_part = (int(filename[3:7]) - offset) // 100 * 10 + (int(filename[3:7]) % 100) // 10
    if num_part in sorted_filenames_corrected:
        sorted_filenames_corrected[num_part].append(filename)

# sorted_filenames_corrected = 
file_names_list_2d = [list(val) for val in sorted_filenames_corrected.values()]
file_names_list_2d = list(zip(*file_names_list_2d))  # Correct transposition



print(file_names_list_2d)
# # Ensure ordering is correct by maintaining the specified order
# df_sorted_filenames = pd.DataFrame.from_dict(sorted_filenames_corrected, orient="index").T

# # Display the corrected sorted groups
# tools.display_dataframe_to_user(name="Reordered Log Groups (Final)", dataframe=df_sorted_filenames)



# In[ ]:


number_of_pq_on_currently_deleted_keys = 100000

selected_columns = \
['File Name',
 'insertion_time_ns _out',
 'rd_time_ns _out',
 ' filter false positive rate _out',
 'point_query_time_on_existing_keys_ns _out',
 'point_query_time_on_deleted_keys_ns _out',
 'all_time_ns _out',
 ' Number Of Total Memory Usage _out',
#  ' Number Of Total Memory Usage Origin Included Timestamp _out',
#  f' (Currently Deleted Keys fixed #PQ = {number_of_pq_on_currently_deleted_keys})  block_read_count _out',
#  ' (Currently Deleted Keys fixed #PQ = 100000)  block_read_byte _out',
#  ' (Currently Deleted Keys fixed #PQ = 100000)  block_read_time _out',
#  ' (Currently Deleted Keys fixed #PQ = 100000)  index_block_read_count  _out',
#  ' (Currently Deleted Keys fixed #PQ = 100000)  filter_block_read_count  _out',
  'fetcher__num_filter_read_count _out',
  'fetcher__num_index_read_count _out',
  'fetcher__num_range_del_read_count _out',
  'fetcher__num_data_read_count _out',
  'fetcher__num_meta_index_read_count _out',
  'fetcher__num_properties_read_count _out',
  'fetcher__num_compression_dict_block_read_count _out',
  'fetcher__num_filter_partition_index_read_count _out',
  'fetcher__num_hash_index_meta_read_count _out',
  'fetcher__num_hash_index_prefixes_read_count _out',
  'fetcher__num_total_block_read_count _out',
]

# In[ ]:


# Convert dictionary to DataFrame
df_structured = pd.DataFrame.from_dict(structured_data, orient="index")
# display(df_structured.columns.to_list())

# Reset index and rename columns
df_structured.reset_index(inplace=True)
df_structured.rename(columns={"index": "File Name"}, inplace=True)

df_structured = df_structured[selected_columns]


print(df_structured["File Name"])
# display(df_structured.columns.to_list())


# In[ ]:


# Initialize a dictionary to store the new structured data
new_df_table = {}

# Iterate through each group ID in sorted order
for file_group in file_names_list_2d:
    for log_id in file_group:
        # Extract the corresponding row from df_structured
        line = df_structured[df_structured["File Name"] == log_id]
        # print(log_id)
        # Store it in the new table
        new_df_table[log_id] = line

# Convert dictionary to a structured DataFrame
# df_new_table = pd.concat(new_df_table.values())
# df_new_table = new_df_table
df_new_table = df_structured

# Save the new structured table
structured_table_path = "./structured_log_data.xlsx"
df_new_table.to_excel(structured_table_path, index=False)

print(df_new_table)
# # Provide download link
# structured_table_path


# In[18]:


# display(df_structured)

# # Save structured data to Excel
# #structured_excel_filename = "/mnt/data/structured_log_data.xlsx"
# structured_excel_filename = "./structured_log_data.xlsx"
# # structured_excel_filename = "./structured_log_data.csv"
# df_structured.to_excel(structured_excel_filename, index=False)

# ## Provide download link
# #structured_excel_filename



# In[ ]:





# In[ ]:





# In[ ]:




