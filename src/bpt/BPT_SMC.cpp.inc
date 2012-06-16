				

//MPRINT("WILL COMPONENTEN VERSCHIEBEN")

				//Auf jeden Fall UVSlideArrays initialisieren
				initUVSlideArrays();
				initSlideArrays();				

				MItMeshVertex	vertIter(fMesh);
				MItMeshPolygon	polyIter(fMesh);
				MItMeshEdge		edgeIter(fMesh);
				
				MIntArray allVtx;
				int l;
				
				simpleLinkedList<MIntArray> connectionsList;	//sync zu linedVtx
				MIntArray					linedVtx;		// hält die IDs der Vtx Typ 1 und 2 (control)
				MIntArray					isControlTwo;	//sync zu linedVtx,ist type 2 control oder nicht
				MIntArray					countArray;

				getAllVertexIndices(allVtx);

				
				MIntArray selVtx = allVtx;	//wird benötigt, damit immer ein komplettes set der ursprünglich gewählten Vtx vorhanden ist


				MIntArray connections; 
				unsigned int count;
				bool	displayError = true;
				bool	onBorder;

				//begin
				for(unsigned int a = 0; a < allVtx.length(); a++)
				{
				
					onBorder = getVtxConnectInfo(allVtx[a],connections,count,selVtx,vertIter,edgeIter);

				
					//wingedVtx ausschließen
					if(connections.length() <= 2 )
					{
						cout<<"Vtx "<<allVtx[a]<<" is a winged- or invalid vertex. It has been excluded."<<endl; 
						

						if(displayError)
						{
							displayError = false;
							MGlobal::displayError("At least one vertex has been invalid. Please see the output window for further information.");
						}

						allVtx.remove(a--);

						continue;
					}


					switch(count)
					{
					case 1:
						{//case 1 und 2 erhalten dieselbe Aktion

						}
					case 2:
						{
							if(onBorder && count > 1)
							{
								allVtx.remove(a--);
								break;
							}

							connectionsList.append(new MIntArray(connections));
							linedVtx.append(allVtx[a]);
							countArray.append(count);

							//bestimmen, ob dieser Vtx type 2 control ist oder nicht
							if(count == 1)
								isControlTwo.append(0);
							else
							{//einen selVtx finden, funzt nur ideal bei quadMesh
								l = connections.length();
								for(int i = 0; i < l; i++)
								{
									if(connections[i] > -1) //-> x >= 0
									{
										if(connections[(i+1)%l] >= 0 || connections[(i-1+l)%l] >= 0)
											isControlTwo.append(1);
										else
											isControlTwo.append(0);

										break;
									}
								}
							}

							

							break;
						}
					case 3:
						{
							//wenn type 3 auch nur 3 connections hat, dann isses quasi type 4 und erhält nur normalSlide
							if(connections.length() == 3)
							{
								addNormalSlide(allVtx[a],vertIter,polyIter);
								
							}
							else
							{	//ansonsten gibt es für diesen Typ eindeutige directions

								processType3(allVtx[a], connections, vertIter,polyIter);

							}
							
							allVtx.remove(a--);

							break;
						}
					case 4:	//wird eventuell nicht gebraucht
						{
							processType4Vertex(allVtx[a],connections,selVtx,vertIter, polyIter);
						
							allVtx.remove(a--);

							break;
						}
					default:
						{
							//nur normalSlide hinzufügen
							addNormalSlide(allVtx[a],vertIter,polyIter);
							allVtx.remove(a--);
							
							
							break;
						}

					}
				
					
					
					
				}

				//jetzt die linedVtx weiter bearbeiten
				MIntArray	isFlipped;	//jeder Vtx, der hierauf gefunden wird, hat ne geflippte direction
				MIntArray	vtxRemovals;
				


				l = linedVtx.length();

				INVIS(helper.printArray(linedVtx, " = linedVtx");)

				MIntArray tmpArray;	//zur aufnahme des return wertes von processLinedVtx

				INVIS(cout<<"GRÖßE ConnectionsList: "<<connectionsList.length()<<endl;)



				if(l != 0)
				{
					for(int i = 0; i < l; i++)
					{
						if(isControlTwo[i] && linedVtx[i] != -1)
						{
							MPRINT("STARTE PROCESS LINED VTX")
							processLinedVtx(MIntArray(1,i), linedVtx, isControlTwo, connectionsList,countArray, isFlipped,vtxRemovals, vertIter, polyIter);					
						}
					}

					MPRINT("Entferne Vtx in Main")

					helper.arrayRemove(allVtx,vtxRemovals);

					INVIS(helper.printArray(allVtx," = allVtx nach remove");)
				}
				
				
				//hier die restlichen vertex bearbeiten
				if(allVtx.length() != 0)
				{
					//hier normale lines bearbeiten
					MPRINT("WILL NORMALE LINES BEARBEITEN")
					processUncontrolledLines(linedVtx, connectionsList,countArray,vertIter, polyIter);
					
				}

				
