package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"sync"
	"time"
)

const (
	maxRetries = 3
	retryDelay = 2 * time.Second
	baseUrl    = "https://github.com/msys2/msys2-archive/releases/download"
	targetDir  = "C:\\msys64\\var\\lib\\pacman\\sync"
)

var databases = []string{
	"clang64",
	"clangarm64",
	"mingw32",
	"mingw64",
	"msys",
	"ucrt64",
}

func downloadFile(url string, filepath string) error {
	var err error
	for i := 0; i < maxRetries; i++ {
		if i > 0 {
			fmt.Printf("Retrying download for %s in %v...\n", url, retryDelay)
			time.Sleep(retryDelay)
		}

		err = attemptDownload(url, filepath)
		if err == nil {
			return nil
		}
		fmt.Printf("Attempt %d failed for %s: %v\n", i+1, url, err)
	}
	return fmt.Errorf("failed to download %s after %d attempts: %w", url, maxRetries, err)
}

func attemptDownload(url string, filepath string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("bad status: %s", resp.Status)
	}

	out, err := os.Create(filepath)
	if err != nil {
		return err
	}
	defer out.Close()

	_, err = io.Copy(out, resp.Body)
	return err
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Usage: %s <archive_date>\n", os.Args[0])
		os.Exit(1)
	}

	archiveDate := os.Args[1]
	archiveBaseUrl := baseUrl + "/" + archiveDate

	if _, err := os.Stat(targetDir); os.IsNotExist(err) {
		fmt.Printf("Target directory %s does not exist. Exiting.\n", targetDir)
		os.Exit(1)
	}

	var wg sync.WaitGroup

	for _, db := range databases {
		filesToDownload := []string{
			fmt.Sprintf("%s.db", db),
			fmt.Sprintf("%s.db.sig", db),
		}

		for _, file := range filesToDownload {
			wg.Add(1)
			go func(fileName string) {
				defer wg.Done()
				fileUrl := fmt.Sprintf("%s/%s", archiveBaseUrl, fileName)
				filePath := filepath.Join(targetDir, fileName)

				fmt.Printf("Downloading %s...\n", fileUrl)
				if err := downloadFile(fileUrl, filePath); err != nil {
					fmt.Printf("Failed to download %s: %v\n", fileUrl, err)
				} else {
					fmt.Printf("Successfully downloaded %s\n", filePath)
				}
			}(file)
		}
	}

	wg.Wait()
	fmt.Println("All downloads completed.")
}
